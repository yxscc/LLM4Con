#include "CCPG/ThreadCreationTree.h"
#include "phasar.h"
#include "PhasarUtil/LLVMAnalyzer.h"
#include "CCPG/AliasChecker.h"
#include "LLMUtil/FindingThreadEntryAgent.h"
#include "PhasarUtil/PhasarPointerAnalysis.h"
#include "PhasarUtil/AnalysisManager.h"

#include <iostream>
#include <regex>
#include <unordered_set>
#include <cxxabi.h>
#include <filesystem>
#include <queue>

using namespace ccpg;
using namespace psr;

// 全局缓存，存储每个函数对应的所有调用链
std::unordered_map<std::pair<const ccpg::Function*, const ccpg::Function*>, std::vector<std::vector<const ccpg::Function*>>, pair_hash> callPathCache;

ThreadCreationTree* ThreadCreationTree::instance = nullptr;

void ThreadCreationTree::addThread(Thread* thread) {
    threads.insert(thread);
}

void ThreadCreationTree::build(){
    const CPG * cpg = this->getCPG();
    CCPG * ccpg = this->getCCPG();

    // create main Thread
    std::queue<std::pair<CCPGNode *, Thread *>> forkQueue;
    FunctionSet entries = ccpg->getEntryFunctions();
    for(ccpg::Function * entry : entries){
        Thread * entryThread = createThread(entry->getFuncNode(), nullptr);
        entryThread->setMainThread(true);
        for(CCPGNode* forkNode : entryThread->getNodesByType(ThreadAPIUtil::TYPE::FORK)){
            forkQueue.push(std::make_pair(forkNode, entryThread));
        }
    }

    int i = 0;
    while(!forkQueue.empty()){

        if(i == 100) break;      //support only 100 abstract threads

        std::pair<CCPGNode *, Thread *> pair = forkQueue.front();
        forkQueue.pop();

        CCPGNode * forkNode = pair.first;
        Thread * parent = pair.second;
        Thread * thread = createThread(forkNode, parent);

        if(thread != nullptr){
            for(CCPGNode* forkNode : thread->getNodesByType(ThreadAPIUtil::TYPE::FORK)){
                forkQueue.push(std::make_pair(forkNode, thread));
            }
        }

        i++;
    } 

    std::unordered_set<Thread * > wrongThread;
    for(Thread * thread : threads){
        if(thread->getParent() == nullptr && thread->getChildren().size() == 0)
        {
            wrongThread.insert(thread);
        }
    }
    for(Thread * thread : wrongThread){
        threads.erase(thread);
    }

    // 暂时移除main线程
    for (Thread* thread : threads) {
        if (thread->isMainThread()) {
            threads.erase(thread);
            break;
        }
    }
}

std::string removeAmpersand(const std::string& str) {
    std::string result;
    for (char ch : str) {
        if (ch != '&') {
            result += ch;
        }
    }
    return result;
}

void ThreadCreationTree::handleJoins(){
    CCPG * ccpg = ThreadCreationTree::getInstance()->getCCPG();
    std::unordered_set<CCPGNode * > joinNodes = ccpg->getNodesByType(ThreadAPIUtil::TYPE::JOIN);
    std::unordered_map<Thread * , CCPGNode *> thread_to_join_map;
    AliasChecker * aliasChecker = AliasChecker::getInstance();

    for(Thread * thread : threads){
        if(thread->getForkNode()->getType() != ThreadAPIUtil::TYPE::FORK){
            continue;
        }
        thread_to_join_map[thread] = nullptr;
    }

    for(CCPGNode * joinNode : joinNodes){
        std::vector<std::pair<Thread*, int>> candidates;

        for(Thread * thread : threads){
            CCPGNode* fork_node = thread->getForkNode();

            if(fork_node->getType() != ThreadAPIUtil::TYPE::FORK){
                continue;
            }

            if(!aliasChecker->isThreadAlias(fork_node, joinNode)){
                continue;
            }

            int score = 0;
            if(joinNode->getFunction() == fork_node->getFunction()){
                score += 2;
            }

            std::string joinThreadName = removeAmpersand(joinNode->getCPGNode()->getArgument(1)->getCode());
            std::string forkThreadName = removeAmpersand(fork_node->getCPGNode()->getArgument(1)->getCode());
            if(joinThreadName == forkThreadName){
                score += 1;
            }

            candidates.emplace_back(thread, score);
            
        }

        if (candidates.empty()) continue;
        std::sort(candidates.begin(), candidates.end(),
            [](const auto& a, const auto& b) {
                    return a.second > b.second;
            });
        Thread* best_match = candidates[0].first;
        
        if (thread_to_join_map[best_match] == nullptr) {
            thread_to_join_map[best_match] = joinNode;
        }

    }

    // Step 3: 补充匹配（为未匹配的线程分配 join 节点）
    for (auto& [thread, join_node] : thread_to_join_map) {
        if (join_node == nullptr) {
            // 选择一个最接近的 join 节点（即使不完全满足条件）
            for (CCPGNode* join_node : joinNodes) {
                if (aliasChecker->isThreadAlias(join_node, thread->getForkNode())) {
                    thread_to_join_map[thread] = join_node;
                    break;
                }
            }
        }
    }

    for (auto& [thread, join_node] : thread_to_join_map) {
        thread->setJoinNode(join_node);
    }

}

Thread * ThreadCreationTree::createThread(CCPGNode* forkNode, Thread* parent){
    CCPG * ccpg = this->getCCPG();

    if(forkNode->getType() == ThreadAPIUtil::TYPE::FORK){
        int here = 1;
    }

    Thread* thread = new Thread();
    thread->setParent(parent);
    thread->setId(threads.size());
    thread->setForkNode(forkNode);

    CCPGNode * functionNode;
    // create thread for main function
    if(parent == NULL){
        functionNode = forkNode;
    }

    // create thread for fork 
    else{
        parent->addChild(thread);
        Node * functionCPGNode = findThreadEntryInCPG(forkNode);
        if(functionCPGNode == nullptr){
            return nullptr;
        }
        functionNode = ccpg->createCCPGNode(functionCPGNode);
    }

    ccpg::Function * threadFunction = ccpg->createFunction(functionNode);
    thread->setThreadMainFunction(threadFunction);

    std::queue<ccpg::Function *> functionQueue;
    functionQueue.push(threadFunction);
    std::unordered_set<ccpg::Function *> visited;

    // create function for each call node
    while (!functionQueue.empty()) {
        ccpg::Function * function = functionQueue.front();
        functionQueue.pop();
        if(visited.find(function) != visited.end()){
            continue;
        }
        visited.insert(function);

        thread->addFunction(function);

        for(CCPGNode * objectInitNode : function->getNodesByType(ThreadAPIUtil::TYPE::OBJECT_INIT)){

            CCPGEdge* edge = ccpg->hasCallEdge(objectInitNode);
            if (edge != nullptr) {
                thread->addEdge(edge);
                ccpg::Function * f = ccpg->getFunctionByCCPGNode(edge->getDst());
                functionQueue.push(f);
                continue; 
            }
            CCPGNode * callee = ccpg->findCalleeByCaller(objectInitNode);
            if(callee == nullptr){
                continue;
            }
            ccpg::Function * f = ccpg->createFunction(callee);
            functionQueue.push(f);
            edge = ccpg->createCCPGEdge(objectInitNode, callee);
            edge->setType(CCPGEdge::EdgeType::CALL);
            ccpg->addEdge(edge);
            thread->addEdge(edge);
            
        }

        for(CCPGNode * callNode : function->getNodesByType(ThreadAPIUtil::TYPE::OTHER_CALL)){
            CCPGEdge* edge = ccpg->hasCallEdge(callNode);
            if (edge != nullptr) {
                thread->addEdge(edge);
                ccpg::Function * f = ccpg->getFunctionByCCPGNode(edge->getDst());
                functionQueue.push(f);
                continue;
            }

            CCPGNode * callee = ccpg->findCalleeByCaller(callNode);
            if(callee == nullptr){
                continue;
            }
            ccpg::Function * f = ccpg->createFunction(callee);
            functionQueue.push(f);
            
            edge = ccpg->createCCPGEdge(callNode, callee);
            edge->setType(CCPGEdge::EdgeType::CALL);
            ccpg->addEdge(edge);
            thread->addEdge(edge);
        }

    }

    for(ccpg::Function * function : thread->getFunctions()){
        for(CCPGNode * node : function->getNodes()){
            thread->addNode(node);
        }
        for(CCPGEdge * edge : function->getEdges()){
            thread->addEdge(edge);
        }
    }
    addThread(thread);
    return thread;
}

Thread* ThreadCreationTree::getThreadById(int thread_id){
    for (Thread* thread : threads) {
        if (thread->getId() == thread_id) {
            return thread;
        }
    }
    return nullptr;
}

Node* ThreadCreationTree::findThreadEntryInCPG(CCPGNode* forkNode){
    const CPG * cpg = getCPG();
    Node* fork = forkNode->getCPGNode();
    Node* methodArgument = fork->getArgument(3);
    
    Node * method = nullptr;

    method = findThreadEntryByArg(methodArgument);

    if(method == nullptr){
        method = findThreadEntryByLLM(forkNode);
    }    

    return method;
}

Node * ThreadCreationTree::findThreadEntryByLLM(CCPGNode* forkNode){
    const CPG * cpg = getCPG();
    Node* fork = forkNode->getCPGNode();

    llm_client::FindingThreadEntryAgent agent(this->ccpg, llm_client::LLMClient::get_instance());
    int result = agent.find_thread_entry(forkNode);
    std::string result_str = std::to_string(result);
    const char * node_id = result_str.c_str();
    return cpg->findNode(node_id);
}

Node* ThreadCreationTree::findThreadEntryByArg(Node * methodArgument){
    const CPG * cpg = getCPG();

    if(methodArgument == nullptr){
        return nullptr;
    }

    if(methodArgument->getType() != "Method_ref"){
        for(Edge* edge : methodArgument->outEdges){
            Node* node = edge->getToNode();
            if(node->getType() == "Method_ref"){
                methodArgument = node;
                break;
            }
        }
    }

    std::string name = methodArgument->getName();
    if(name == "") name = methodArgument->properties.at("CODE");

    Node* method = cpg->findMethod(name);

    return method;

}

void Thread::addFunction(ccpg::Function* function){
    functions.insert(function);
    nodes.insert(function->getNodes().begin(), function->getNodes().end());
    edges.insert(function->getEdges().begin(), function->getEdges().end());
    for(CCPGNode* node : function->getNodes()){
        node->addRelevantThread(this);
    }
}

std::vector<std::vector<const ccpg::Function*>> 
getAllCallPaths(const ccpg::Function* target, const ccpg::Function* root) {
    // 检查缓存
    auto cacheKey = std::make_pair(root, target);
    if (callPathCache.count(cacheKey)) {
        return callPathCache.at(cacheKey);
    }

    // 结果存储
    std::vector<std::vector<const ccpg::Function*>> result;

    // 检查输入有效性
    if (!target || !root) {
        return result; // 返回空结果
    }

    // 递归辅助函数
    std::function<void(const ccpg::Function*, std::vector<const ccpg::Function*>, 
                     std::unordered_set<const ccpg::Function*>&)>
dfs = [&](const ccpg::Function* current, 
              std::vector<const ccpg::Function*> currentPath,
              std::unordered_set<const ccpg::Function*>& visited) {
        // 如果当前节点是根节点，保存路径
        if (current == root) {
            currentPath.push_back(current);
            // 反转路径，使其从 root 到 target
            std::reverse(currentPath.begin(), currentPath.end());
            result.push_back(currentPath);
            return;
        }

        // 如果当前节点已访问过，跳过（避免循环）
        if (visited.count(current)) {
            return;
        }

        // 标记当前节点为已访问
        visited.insert(current);
        currentPath.push_back(current);

        // 遍历所有调用者（caller）
        for (const auto* caller : current->getCallers()) {
            dfs(caller, currentPath, visited); // 递归
        }

        // 回溯：移除当前节点
        visited.erase(current);
    };

    // 初始化并开始递归
    std::unordered_set<const ccpg::Function*> visited;
    dfs(target, {}, visited);

    // 将结果存入缓存
    callPathCache[cacheKey] = result;

    return result;
}

// 辅助函数：判断调用链 callChain1 是否在 callChain2 之前执行
bool isBeforeInCallChains(const std::vector<const ccpg::Function*>& callChain1,
    const std::vector<const ccpg::Function*>& callChain2) {
    // 找到最后一个公共祖先函数
    size_t commonDepth = 0;
    while (commonDepth < callChain1.size() && commonDepth < callChain2.size() &&
        callChain1[commonDepth] == callChain2[commonDepth]) {
        commonDepth++;
    }

    if (commonDepth == 0) return false; // 无公共祖先（理论上不可能）

    // 情况1：callChain1 和 callChain2 在同一函数中
    if (commonDepth == callChain1.size() && commonDepth == callChain2.size()) {
        return false; // 同一函数中，无法确定顺序
    }

    // 情况2：callChain1 位于公共函数中，callChain2 在子调用链中
    if (commonDepth == callChain1.size()) {
        const ccpg::Function* nextFunc2 = callChain2[commonDepth];
        const ccpg::Function* commonFunc = callChain1[commonDepth - 1];
        int callOrder2 = commonFunc->getCallOrder(nextFunc2);
        return (callOrder2 != -1);
    }

    // 情况3：callChain2 位于公共函数中，callChain1 在子调用链中
    if (commonDepth == callChain2.size()) {
        const ccpg::Function* nextFunc1 = callChain1[commonDepth];
        const ccpg::Function* commonFunc = callChain2[commonDepth - 1];
        int callOrder1 = commonFunc->getCallOrder(nextFunc1);
        return (callOrder1 != -1);
    }

    // 情况4：callChain1 和 callChain2 位于公共函数的不同子调用链中
    const ccpg::Function* nextFunc1 = callChain1[commonDepth];
    const ccpg::Function* nextFunc2 = callChain2[commonDepth];
    const ccpg::Function* commonFunc = callChain1[commonDepth - 1];
    int callOrder1 = commonFunc->getCallOrder(nextFunc1);
    int callOrder2 = commonFunc->getCallOrder(nextFunc2);

    return (callOrder1 != -1 && callOrder2 != -1) && (callOrder1 < callOrder2);
}





// Helper function to check reachability within a thread's CFG
bool  isReachable(CCPGNode* start, CCPGNode* end, Thread* thread) {
    if (!start || !end || !thread) return false;
    if (start == end) return true;

    std::queue<CCPGNode*> worklist;
    std::unordered_set<CCPGNode*> visited;

    worklist.push(start);
    visited.insert(start);

    const auto& thread_nodes = thread->getNodes();

    while (!worklist.empty()) {
        CCPGNode* current = worklist.front();
        worklist.pop();

        for (CCPGEdge* edge : current->getOutEdges()) {
            CCPGNode* next = edge->getDst();

            if (next == end) {
                return true;
            }

            // Only explore nodes within the same thread that haven't been visited
            if (thread_nodes.count(next) && visited.find(next) == visited.end()) {
                visited.insert(next);
                worklist.push(next);
            }
        }
    }
    return false;
}


bool ThreadCreationTree::mayHappenInParallel(Thread * t1, Thread * t2) {
    // Generate an ordered pair for the cache key
    auto cacheKey = (t1 <= t2) ? std::make_pair(t1, t2) : std::make_pair(t2, t1);

    // Check cache first
    if (mayHappenInParallelCache.count(cacheKey)) {
        return mayHappenInParallelCache.at(cacheKey);
    }

    assert(t1->getParent() == t2->getParent() && "mayHappenInParallel should only be called on sibling threads");
    Thread* parent = t1->getParent();
    if (!parent) {
        mayHappenInParallelCache[cacheKey] = false;
        return false;
    }

    CCPGNode * forkNode1 = t1->getForkNode();
    CCPGNode * forkNode2 = t2->getForkNode();
    CCPGNode * joinNode1 = t1->getJoinNode();
    CCPGNode * joinNode2 = t2->getJoinNode();

    // t1 happens-before t2 if join(t1) is reachable to fork(t2) in the parent thread's CFG.
    // If t1 is never joined, it cannot have a happens-before relationship with the creation of t2.
    bool t1HBt2 = (joinNode1 != nullptr) && isReachable(joinNode1, forkNode2, parent);

    // t2 happens-before t1 if join(t2) is reachable to fork(t1) in the parent thread's CFG.
    bool t2HBt1 = (joinNode2 != nullptr) && isReachable(joinNode2, forkNode1, parent);

    // They can happen in parallel if neither happens-before the other.
    bool result = !t1HBt2 && !t2HBt1;
    mayHappenInParallelCache[cacheKey] = result;
    return result;
}

void ThreadCreationTree::countParallelThreadPairs(){
    // Using a vector is cleaner for iterating over unique pairs
    std::vector<Thread*> threads_vec(threads.begin(), threads.end());
    
    for (size_t i = 0; i < threads_vec.size(); ++i) {
        for (size_t j = i + 1; j < threads_vec.size(); ++j) {
            Thread* t1 = threads_vec[i];
            Thread* t2 = threads_vec[j];

            // Check for descendant relationship first, as it's the most specific
            if (isDescendant(t1, t2)) { // t1 is a descendant of t2
                addParallelThreadPairs(t1, t2, "descendant");
                continue; // Found relationship, move to next pair
            }
            if (isDescendant(t2, t1)) { // t2 is a descendant of t1
                addParallelThreadPairs(t2, t1, "descendant");
                continue; // Found relationship, move to next pair
            }

            // Check for sibling relationship
            // A quick check for a common parent before the more expensive isSibling call
            if (t1->getParent() != nullptr && t1->getParent() == t2->getParent()) {
                if (isSibling(t1, t2)) {
                    addParallelThreadPairs(t1, t2, "sibling");
                } else {
                    // If isSibling is false, they are on different branches, but still indirect siblings
                    addParallelThreadPairs(t1, t2, "indirect sibling");
                }
                continue; // Found relationship, move to next pair
            }

            // Finally, check for indirect sibling relationship (common ancestor)
            // This is the most general case
            if (isIndirectSibling(t1, t2)) {
                addParallelThreadPairs(t1, t2, "indirect sibling");
            }
        }
    }
}

// 判断线程 t1 是否是 t2 的后代
bool ThreadCreationTree::isDescendant(Thread* t1, Thread* t2) {
    Thread* parent = t1->getParent();
    while (parent != nullptr) {
        if (parent == t2) {
            return true;
        }
        parent = parent->getParent();
    }
    return false;
}

bool ThreadCreationTree::mayThreadsRunConcurrently(Thread* t1, Thread* t2) {
    if (t1 == t2) return false;

    auto cacheKey = (t1 <= t2) ? std::make_pair(t1, t2) : std::make_pair(t2, t1);
    if (concurrencyCache.count(cacheKey)) {
        return concurrencyCache.at(cacheKey);
    }

    bool result = false;

    // Case 1: 后代关系。后代线程总是可以与其祖先线程并发。
    if (isDescendant(t1, t2) || isDescendant(t2, t1)) {
        result = true;
    } else {
    // Case 2: 兄弟或间接兄弟关系。
        Thread* p1 = t1->getParent();
        Thread* p2 = t2->getParent();
        
        if (p1 && p1 == p2) { // 直接兄弟
            result = mayHappenInParallel(t1, t2);
        } else if (p1 && p2) { // 间接兄弟（堂兄弟等）
            // 递归地检查它们的父线程是否并发
            result = mayThreadsRunConcurrently(p1, p2);
        }
        // 如果没有共同的父节点（例如，两个主线程），则它们不并发
    }

    concurrencyCache[cacheKey] = result;
    return result;
}

// 补全最后一部分：查找最近公共祖先节点并判断分支类型
CCPGNode* findLCA(CCPGNode* n1, CCPGNode* n2) {
    if (!n1 || !n2) return nullptr;
    if (n1 == n2) return n1;

    // 1. 从 n1 向上遍历，记录所有祖先（包括n1自身）
    std::unordered_set<CCPGNode*> ancestors_of_n1;
    std::queue<CCPGNode*> q;
    q.push(n1);
    
    // 假设节点都在同一个函数内，如果不在，需要额外处理
    ccpg::Function* func = n1->getFunction(); 
    if (!func || func != n2->getFunction()) {
        // 如果不在同一个函数，LCA逻辑会更复杂，
        // 当前假设它们在同一个父线程的CFG内，因此在同一个函数内。
        // 如果这个假设不成立，需要返回或采用更复杂的逻辑。
        return nullptr; 
    }
    
    std::unordered_set<CCPGNode*> visited_in_bfs;
    visited_in_bfs.insert(n1);

    while (!q.empty()) {
        CCPGNode* current = q.front();
        q.pop();
        ancestors_of_n1.insert(current);

        for (CCPGEdge* edge : current->getInEdges()) {
            CCPGNode* parent = edge->getSrc();
            // 确保父节点在同一个函数内，并且没有被访问过
            if (func->getNodes().count(parent) && visited_in_bfs.find(parent) == visited_in_bfs.end()) {
                visited_in_bfs.insert(parent);
                q.push(parent);
            }
        }
    }

    // 2. 从 n2 向上遍历，遇到的第一个在 n1 祖先集合中的节点即为LCA
    q = {}; // Clear the queue
    q.push(n2);
    visited_in_bfs.clear();
    visited_in_bfs.insert(n2);

    while (!q.empty()) {
        CCPGNode* current = q.front();
        q.pop();

        if (ancestors_of_n1.count(current)) {
            return current; // 找到了LCA
        }

        for (CCPGEdge* edge : current->getInEdges()) {
            CCPGNode* parent = edge->getSrc();
            if (func->getNodes().count(parent) && visited_in_bfs.find(parent) == visited_in_bfs.end()) {
                visited_in_bfs.insert(parent);
                q.push(parent);
            }
        }
    }

    return nullptr; // 没有找到公共祖先
}

// 判断线程 t1 和 t2 是否是兄弟关系（即它们有相同的父线程）
bool ThreadCreationTree::isSibling(Thread* t1, Thread* t2) {
    Thread* parent1 = t1->getParent();
    Thread* parent2 = t2->getParent();
    if(parent1 == parent2 && parent1 != nullptr){
        ccpg::Function * pMainF = parent1->getThreadMainFunction();
        std::vector<std::vector<const ccpg::Function*>> callPaths1 = getAllCallPaths(t1->getForkNode()->getFunction(), pMainF);
        std::vector<std::vector<const ccpg::Function*>> callPaths2 = getAllCallPaths(t2->getForkNode()->getFunction(), pMainF);
        std::vector<const ccpg::Function*> callPath1 = callPaths1[0];
        std::vector<const ccpg::Function*> callPath2 = callPaths2[0];

        // 找到两个调用链的最后一个公共祖先函数
        size_t commonDepth = 0;
        while (commonDepth < callPath1.size() && commonDepth < callPath2.size() &&
            callPath1[commonDepth] == callPath2[commonDepth]) {
            commonDepth++;
        }
        
        // 在相同祖先函数中分别找到callPath1和callPath2的调用点
        const ccpg::Function* commonFunc = callPath1[commonDepth - 1];
        CCPGNode * node1;
        CCPGNode * node2;
        if(commonDepth == callPath1.size()){
            node1 = t1->getForkNode();
        }
        else{
            const ccpg::Function* nextFunc1 = callPath1[commonDepth];
            node1 = ccpg->getCallSiteInFunction(commonFunc, nextFunc1);
        }
        if(commonDepth == callPath2.size()){
            node2 = t2->getForkNode();
        }
        else{
            const ccpg::Function* nextFunc2 = callPath2[commonDepth];
            node2 = ccpg->getCallSiteInFunction(commonFunc, nextFunc2);
        }

        // 根据控制流判断这两个调用点的公共祖先是否为Branch类型，如果是，说明不在同一分支中
        // 在原有代码尾部添加以下逻辑
        CCPGNode* lca_node = findLCA(node1, node2);
        if (lca_node && lca_node->getType() == ThreadAPIUtil::TYPE::BRANCH) {
            return false; // 调用��位于分支节点的不同分支
        }
        if (lca_node && lca_node->getType() != ThreadAPIUtil::TYPE::BRANCH){
            return true;
        }
        if (!lca_node) {
            return false;
        }

    }  
    return false;
}

// 判断线程 t1 和 t2 是否是间接兄弟关系（即它们有共同的祖先线程）
bool ThreadCreationTree::isIndirectSibling(Thread* t1, Thread* t2) {
    // 获取 t1 和 t2 的父链
    std::unordered_set<Thread*> t1_parents;
    std::unordered_set<Thread*> t2_parents;

    // 获取 t1 的父链
    Thread* parent1 = t1->getParent();
    while (parent1 != nullptr) {
        t1_parents.insert(parent1);
        parent1 = parent1->getParent();
    }

    // 获取 t2 的父链
    Thread* parent2 = t2->getParent();
    while (parent2 != nullptr) {
        t2_parents.insert(parent2);
        parent2 = parent2->getParent();
    }

    // 检查 t1 和 t2 是否有共同的祖父线程
    for (Thread* p1 : t1_parents) {
        for (Thread* p2 : t2_parents) {
            if (p1 == p2) {
                return true; // 存在共同的父线程
            }
        }
    }
    return false;
}

std::pair<std::unordered_map<NodeLoc, Context, NodeLocHash>, std::unordered_map<NodeLoc, Context, NodeLocHash>>
ThreadCreationTree::computeParallelLocs(Thread* t1, Thread* t2) {
    std::unordered_map<NodeLoc, Context, NodeLocHash> parallelLocs1;
    std::unordered_map<NodeLoc, Context, NodeLocHash> parallelLocs2;

    if (!mayThreadsRunConcurrently(t1, t2)) {
        return {parallelLocs1, parallelLocs2};
    }

    // 为每个线程的所有NodeLoc预先生成一次上下文，避免重复计算
    auto build_all_contexts = [this](Thread* thread, std::unordered_map<NodeLoc, Context, NodeLocHash>& locs) {
        ccpg::Function* mainFunc = thread->getThreadMainFunction();
        if (!mainFunc) return;

        for (CCPGNode* node : thread->getNodes()) {
            const NodeLoc& loc = node->getNodeLoc();
            if (loc.getLineNumber() > 0 && locs.find(loc) == locs.end()) {
                ccpg::Function* currentFunc = node->getFunction();
                if (currentFunc) {
                    Context ctx;
                    if(thread->getForkNode()) ctx.push(thread->getForkNode());
                    
                    std::vector<CCPGNode*> path_nodes = findCallPath(mainFunc, currentFunc, thread);
                    for(CCPGNode* p_node : path_nodes) {
                        ctx.push(p_node);
                    }
                    locs[loc] = ctx;
                }
            }
        }
    };
    
    // Case 1: 兄弟关系
    if (t1->getParent() && t1->getParent() == t2->getParent()) {
        build_all_contexts(t1, parallelLocs1);
        build_all_contexts(t2, parallelLocs2);
        return {parallelLocs1, parallelLocs2};
    }

    // Case 2: 后代关系 (假设 t2 是 t1 的后代)
    Thread *parent = nullptr, *child = nullptr;
    if (isDescendant(t2, t1)) {
        parent = t1;
        child = t2;
    } else if (isDescendant(t1, t2)) {
        parent = t2;
        child = t1;
    }
    
    if (parent && child) {
        // 子线程的所有位置都是并发的
        build_all_contexts(child, (child == t1) ? parallelLocs1 : parallelLocs2);

        // 计算父线程的并发范围
        CCPGNode* forkNode = child->getForkNode();
        CCPGNode* joinNode = child->getJoinNode();

        CCPGNodeSet concurrentScope;
        CCPGNodeSet reachable_from_fork = getReachableNodes(forkNode, parent, true);

        if (joinNode) {
            CCPGNodeSet can_reach_join = getReachableNodes(joinNode, parent, false);
            // 计算交集
            for (CCPGNode* node : reachable_from_fork) {
                if (can_reach_join.count(node)) {
                    concurrentScope.insert(node);
                }
            }
        } else {
            // 如果没有join，则fork之后的所有可达节点都并发
            concurrentScope = reachable_from_fork;
        }

        // 只为并发范围内的父线程节点构建上下文
        ccpg::Function* mainFunc = parent->getThreadMainFunction();
        auto& parentLocs = (parent == t1) ? parallelLocs1 : parallelLocs2;

        for (CCPGNode* node : concurrentScope) {
            const NodeLoc& loc = node->getNodeLoc();
            if (loc.getLineNumber() > 0 && parentLocs.find(loc) == parentLocs.end()) {
                 ccpg::Function* currentFunc = node->getFunction();
                if (currentFunc) {
                    Context ctx;
                    if(parent->getForkNode()) ctx.push(parent->getForkNode());
                     std::vector<CCPGNode*> path_nodes = findCallPath(mainFunc, currentFunc, parent);
                    for(CCPGNode* p_node : path_nodes){
                       ctx.push(p_node);
                    }
                    parentLocs[loc] = ctx;
                }
            }
        }
        return {parallelLocs1, parallelLocs2};
    }

    return {parallelLocs1, parallelLocs2}; // 默认返回空
}

std::pair<
    std::unordered_map<NodeLoc, Context, NodeLocHash>,
    std::unordered_map<NodeLoc, Context, NodeLocHash>
> ThreadCreationTree::getParallelLocs(Thread * t1, Thread * t2) const {
    return parallelLocCache.getParallelLocs(t1, t2);
}

std::vector<Context> Thread::getIntraThreadContext(NodeLoc loc) {
    
    CCPG * ccpg = ThreadCreationTree::getInstance()->getCCPG();
    std::vector<Context> result;

    ccpg::Function* mainFunc = this->getThreadMainFunction();
    CCPGNodeSet nodes = ccpg->getNodesByLoc(loc);
    const CCPGNode* node = *nodes.begin();
    ccpg::Function* function = node->getFunction();
    std::vector<std::vector<const ccpg::Function*>> callChains = getAllCallPaths(function, mainFunc);

    // 为每条有效调用路径生成上下文
    for (const auto& chain : callChains) {
        Context ctx;
        ctx.push(forkNode);  // 固定首元素
        
        const ccpg::Function* lastFunc = mainFunc;
        // 将调用链转换为入口节点序列
        for (const auto* func : chain) {
            if(func == mainFunc){
                continue;
            }
            for(CCPGNode * callSite : func->getCallSites()){
                if(callSite->getFunction() == lastFunc){
                    ctx.push(callSite);
                    lastFunc = func;
                    break;
                }
            }
        }
        
        // 去重处理（如果不同路径产生相同上下文）
        if (std::find(result.begin(), result.end(), ctx) == result.end()) {
            result.emplace_back(std::move(ctx));
        }
    }

    return result;
}

std::vector<CCPGNode*> ThreadCreationTree::findCallPath(ccpg::Function* startFunc, ccpg::Function* endFunc, Thread* thread) {
    if (startFunc == endFunc) {
        return {startFunc->getFuncNode()};
    }
    std::queue<std::vector<CCPGNode*>> q;
    q.push({startFunc->getFuncNode()});
    std::unordered_set<ccpg::Function*> visited;
    visited.insert(startFunc);

    while (!q.empty()) {
        std::vector<CCPGNode*> path = q.front();
        q.pop();
        ccpg::Function* lastFunc = path.back()->getFunction();
        if (!lastFunc) continue;

        for (CCPGNode* node : lastFunc->getNodes()) {
            if (node->isCallSite()) {
                CCPGEdge* callEdge = ccpg->hasCallEdge(node);
                if (callEdge && callEdge->getDst()) {
                    ccpg::Function* calleeFunc = callEdge->getDst()->getFunction();
                    if (calleeFunc && thread->getFunctions().count(calleeFunc) && visited.find(calleeFunc) == visited.end()) {
                        std::vector<CCPGNode*> new_path = path;
                        new_path.push_back(node);
                        if (calleeFunc == endFunc) {
                            return new_path;
                        }
                        visited.insert(calleeFunc);
                        q.push(new_path);
                    }
                }
            }
        }
    }
    return {}; // 未找到路径
}

CCPGNodeSet ThreadCreationTree::getReachableNodes(CCPGNode* startNode, Thread* thread, bool forward) {
    CCPGNodeSet reachable;
    if (!startNode || !thread || !thread->getNodes().count(startNode)) {
        return reachable;
    }

    std::queue<CCPGNode*> worklist;
    worklist.push(startNode);
    reachable.insert(startNode);

    while (!worklist.empty()) {
        CCPGNode* current = worklist.front();
        worklist.pop();
        
        const auto& edges = forward ? current->getOutEdges() : current->getInEdges();
        for (CCPGEdge* edge : edges) {
            CCPGNode* neighbor = forward ? edge->getDst() : edge->getSrc();
            // 确保邻居节点属于同一个线程，并且之前未访问过
            if (thread->getNodes().count(neighbor) && reachable.find(neighbor) == reachable.end()) {
                reachable.insert(neighbor);
                worklist.push(neighbor);
            }
        }
    }
    return reachable;
}

std::set<const llvm::Value*> ThreadCreationTree::collectCandidateSharedObjects() const {

    if (candidateSharedObjectsCache.has_value()) {
        return *candidateSharedObjectsCache;
    }

    std::set<const llvm::Value*> candidateObjects;
    auto* pa = static_cast<PhasarPointerAnalysis*>(AnalysisManager::getInstance()->getPointerAnalyzer());
    if (!pa) {
        return candidateObjects;
    }

    // 1. 添加所有全局变量
    auto globals = pa->getAllGlobalVariables();
    std::cout << "[DEBUG PRINT] Phase 1: Found " << globals.size() << " global variables as candidate shared objects." << std::endl;
    for (const auto* gv : globals) {
        if (gv->isConstant()) {
            continue;
        }
        candidateObjects.insert(gv);
    }

    // 2. 添加所有线程的入口函数参数
    std::cout << "[DEBUG PRINT] Phase 1: Analyzing thread arguments..." << std::endl;
    for (const auto& thread : this->getThreads()) {
        if (thread->getThreadMainFunction() && thread->getThreadMainFunction()->getLLVMFunction()) {
            for (const auto& arg : thread->getThreadMainFunction()->getLLVMFunction()->args()) {
                candidateObjects.insert(&arg);
                std::cout << "  -> Found thread arg in function '" << thread->getThreadMainFunction()->getLLVMFunction()->getName().str() << "' as a candidate." << std::endl;
            }
        }
    }
    std::cout << "[DEBUG PRINT] Phase 1: Total candidate shared objects found: " << candidateObjects.size() << std::endl;

    candidateSharedObjectsCache = candidateObjects;

    return candidateObjects;
}


void ThreadCreationTree::printThreadCreationTree(fs::path outputDir) const {

    std::ofstream file(outputDir / "thread-creation-tree.dot");

    std::ostringstream dot;
    dot << "digraph ThreadCreationTree {\n";
    dot << "  node [shape=box];\n";

    // 遍历所有线程
    for (Thread* thread : threads) {
        thread->addThreadToDot(dot);
    }

    // 添加线程之间的父子关系
    for (Thread* thread : threads) {
        if (thread->getParent()) {
            dot << "  Thread_" << thread->getParent()->getId() << " -> Thread_" << thread->getId() << ";\n";
        }
    }

    dot << "}\n";
    file << dot.str();
    file.close();
}

void Thread::addThreadToDot(std::ostringstream& dot) {
    // 获取线程的详细信息
    int threadId = getId();
    std::string threadFile = getThreadMainFunction()->getFuncNode()->getNodeLoc().getFileName();
    int threadLine = getThreadMainFunction()->getFuncNode()->getNodeLoc().getLineNumber();
    CCPGNode* forkNode = getForkNode();
    std::string forkFile = forkNode ? forkNode->getNodeLoc().getFileName() : "N/A";
    int forkLine = forkNode ? forkNode->getNodeLoc().getLineNumber() : -1;
    
    // 构建调用链
    std::string callChain = "";
    if (getParent() != nullptr) {
        std::vector<std::vector<const ccpg::Function*>> callChains = getAllCallPaths(getForkNode()->getFunction(), getParent()->getThreadMainFunction());
        for (const auto& chain : callChains) {
            for (size_t i = 0; i < chain.size(); ++i) {
                const ccpg::Function* func = chain[i];
                std::string funcName = func->getFuncNode()->getCPGNode()->getName();
                int funcLine = func->getFuncNode()->getNodeLoc().getLineNumber();
                callChain += funcName + ":" + std::to_string(funcLine);
                if (i < chain.size() - 1) {
                    callChain += " -> ";
                }
            }
            if (!callChain.empty()) {
                callChain += "\n"; // 换行分隔不同的调用链
            }
        }
    }

    // 构建节点的标签
    std::ostringstream label;
    label << "Thread ID: " << threadId << "\n"
          << "File: " << threadFile << "\n"
          << "Line: " << threadLine << "\n"
          << "Fork File: " << forkFile << "\n"
          << "Fork Line: " << forkLine << "\n"
          << "Call Chain: " << (callChain.empty() ? "N/A" : callChain);

    // 添加节点到 DOT
    dot << "  Thread_" << threadId << " [label=\"" << label.str() << "\"];\n";
}

std::pair<
    std::unordered_map<NodeLoc, Context, NodeLocHash>,
    std::unordered_map<NodeLoc, Context, NodeLocHash>
> ParallelLocCache::getParallelLocs(Thread* t1, Thread* t2) const {
    // 1. 生成一个规范的、排序后的键，用于缓存查找和存储
    const auto key = makeOrderedPair(t1, t2);

    // 2. 检查缓存中是否已有结果
    auto it = cache.find(key);
    if (it != cache.end()) {
        // 缓存命中。现在根据调用者的原始顺序返回结果。
        if (key.first == t1) {
            // 调用顺序是 (t1, t2)，与键的顺序一致，直接返回
            return it->second;
        }
        // 调用顺序是 (t2, t1)，与键的顺序相反，交换结果后返回
        return { it->second.second, it->second.first };
    }

    // 3. 缓存未命中：使用规范的、排序后的键来调用计算函数
    //    这可以确保 `result.first` 总是对应 `key.first`，`result.second` 总是对应 `key.second`
    auto result = ThreadCreationTree::getInstance()->computeParallelLocs(key.first, key.second);
    auto emplaceResult = cache.emplace(key, result);

    // 4. 根据调用者的原始顺序返回新计算的结果
    if (key.first == t1) {
        // 调用顺序是 (t1, t2)，与键的顺序一致，直接返回新计算的结果
        return emplaceResult.first->second;
    }
    // 调用顺序是 (t2, t1)，与键的顺序相反，交换新计算的结果后返回
    return { emplaceResult.first->second.second, emplaceResult.first->second.first };
}

MemoryAccessMap ThreadCreationTree::buildMemoryAccessMapForThread(
    Thread* targetThread,
    Thread* otherThread,
    const std::set<const llvm::Value*>& candidates
) const {
    using MemoryAccessMap = std::unordered_map<const llvm::Value*, std::vector<MemoryAccess>>;
    
    MemoryAccessMap accessMap;
    AliasChecker* aliasChecker = AliasChecker::getInstance();
    auto* pa = static_cast<PhasarPointerAnalysis*>(AnalysisManager::getInstance()->getPointerAnalyzer());
    if (!pa) return accessMap;

    // // --- DETAILED LOGGING START ---
    //std::cout << "\n\n// ====================================================================\n"
    //          << "// Building Memory Access Map for Thread " << targetThread->getId() 
    //          << " (Concurrent with Thread " << otherThread->getId() << ")\n"
    //          << "// ====================================================================\n";

    auto [locsForTarget, locsForOther] = this->getParallelLocs(targetThread, otherThread);

    //std::cout << "[DEBUG] Found " << locsForTarget.size() << " concurrent locations to analyze for Thread " << targetThread->getId() << ".\n";
    
    //int loc_count = 1;
    for (const auto& [loc, ctx] : locsForTarget) {
        //std::cout << "\n[DEBUG] " << loc_count++ << "/" << locsForTarget.size() << ": Processing Location: " << loc.toString() << "\n";
        
        auto accesses = aliasChecker->getMemoryAccessesFromLocation(loc, ctx);
        if (accesses.empty()) {
            //std::cout << "  -> No memory accesses found at this location.\n";
            continue;
        }

        //std::cout << "  -> Found " << accesses.size() << " memory access(es) here.\n";
        
        //int access_count = 1;
        for (const auto& access : accesses) {
             std::string access_val_str;
             llvm::raw_string_ostream os(access_val_str);
             access.pointerOperand->print(os);

            //std::cout << "    [" << access_count++ << "/" << accesses.size() << "] Access operand: " << os.str() << "\n";
            //std::cout << "      -> Is Write? " << (access.isWrite ? "Yes" : "No") << "\n";
            
            bool is_shared = false;
             int candidate_count = 1;
            for (const auto* candidate : candidates) {
                 std::string candidate_val_str;
                 llvm::raw_string_ostream os_cand(candidate_val_str);
                //candidate->print(os_cand);

                std::cout << "      -> Comparing with candidate " << candidate_count++ << "/" << candidates.size() << ": " 
                          << (candidate->hasName() ? LLVMAnalyzer::getInstance()->demangle_valueName(candidate->getName().str().c_str()) : "(unnamed)")
                         << " | " << os_cand.str() << "\n";
                
                if (aliasChecker->isAlias(access.pointerOperand, candidate)) {
                    std::cout << "        --> ALIAS MATCH FOUND!\n";
                    is_shared = true;
                    break; 
                }
            }

            if (is_shared) {
                std::cout << "    --> This access is SHARED. Adding to map.\n";
                auto pointsToSet = pa->getPointsToSet(access.pointerOperand);
                if (!pointsToSet.empty()) {
                    const llvm::Value* representative = *pointsToSet.begin();
                    accessMap[representative].push_back(access);
                } else {
                    accessMap[access.pointerOperand].push_back(access);
                }
            } else {
                std::cout << "    --> This access is NOT shared. Skipping.\n";
            }
        }
    }

     std::cout << "\n// ====================================================================\n"
               << "// Finished building map for Thread " << targetThread->getId() 
               << ". Map contains " << accessMap.size() << " unique memory objects.\n"
               << "// ====================================================================\n\n";
    return accessMap;
}

MemoryAccessMap ThreadCreationTree::buildRawMemoryAccessMap(
    Thread* targetThread
) const {
    MemoryAccessMap accessMap;
    AliasChecker* aliasChecker = AliasChecker::getInstance();
    auto* pa = static_cast<PhasarPointerAnalysis*>(AnalysisManager::getInstance()->getPointerAnalyzer());
    if (!pa) return accessMap;

    // Use the existing findAllLocs to get all locations and contexts for the thread
    auto allLocs = targetThread->findAllLocs();

    for (const auto& [loc, ctx] : allLocs) {
        auto accesses = aliasChecker->getMemoryAccessesFromLocation(loc, ctx);
        for (const auto& access : accesses) {
            // Simply add every access found, without checking if it's shared yet.
            // We use the pointer operand itself as the initial key.
            // The alias-based grouping will happen during the intersection phase.
            accessMap[access.pointerOperand].push_back(access);
        }
    }
    return accessMap;
}

std::unordered_map<NodeLoc, Context, NodeLocHash> Thread::findAllLocs() {
    std::unordered_map<NodeLoc, Context, NodeLocHash> locs;
    ThreadCreationTree* tct = ThreadCreationTree::getInstance();
    ccpg::Function* mainFunc = this->getThreadMainFunction();
    if (!mainFunc) return locs;

    for (CCPGNode* node : this->getNodes()) {
        const NodeLoc& loc = node->getNodeLoc();
        // Ensure the location is valid and we haven't processed it yet
        if (loc.getLineNumber() > 0 && locs.find(loc) == locs.end()) {
            ccpg::Function* currentFunc = node->getFunction();
            if (currentFunc) {
                Context ctx;
                if(this->getForkNode()) {
                    ctx.push(this->getForkNode());
                }
                
                // Find the call path from the thread's main function to the node's function
                std::vector<CCPGNode*> path_nodes = tct->findCallPath(mainFunc, currentFunc, this);
                for(CCPGNode* p_node : path_nodes){
                   ctx.push(p_node);
                }
                locs[loc] = ctx;
            }
        }
    }
    return locs;
}