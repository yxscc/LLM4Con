#include "phasar.h"
#include <cxxabi.h>
#include <regex>
#include <filesystem>
#include <unordered_set>
#include <set>
#include <fstream>
#include <tuple>
#include <queue>
#include <iomanip>
#include <sstream>
#include <limits>
#include <cstring>
#include <cstdlib>
#include <vector>

#include "CCPG/HB.h"
#include "CCPG/ManualEntryConfig.h"
#include "PhasarUtil/LLVMAnalyzer.h"
#include "CCPG/AliasChecker.h"
#include "CCPG/LSAnalysis.h"
#include "Util/ExecutionTimer.h"
#include "PhasarUtil/AnalysisManager.h"
#include "PhasarUtil/PhasarPointerAnalysis.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/DebugInfoMetadata.h"

using namespace ccpg;
using namespace psr;

namespace fs = std::filesystem;

void handleContext(CCPGNode * caller, ccpg::Function * f);



CCPGEdge * CCPG::hasCallEdge(CCPGNode * node){
    for(CCPGEdge * edge : node->getOutEdges()){
        if(edge->getType() == CCPGEdge::EdgeType::CALL){
            return edge;
        }
    }
    return nullptr;
}

bool CCPG::hasHBEdge(CCPGNode * node){
    for(CCPGEdge * edge : node->getOutEdges()){
        if(edge->getType() == CCPGEdge::EdgeType::HB){
            return true;
        }
    }
    return false;
}


CCPGNode * CCPG::getCallSiteInFunction(const ccpg::Function * caller, const ccpg::Function * callee){
    CCPGNodeSet callSites = callee->getCallSites();
    for(CCPGNode * node : callSites){
        if(node->getFunction() == caller){
            return node;
        }
    }
    return nullptr;
}

void handleContext(CCPGNode * caller, ccpg::Function * f){
    Function * callerFunction = caller->getFunction();
    ContextSet contextSet = callerFunction->getContextSet();
    for(Context * context : contextSet){
        if(context->contains(caller)){
            continue;
        }
        f->addContext(context->extend(caller));
    }
}

// Push call stacks from the thread roots down the call graph until nothing
// changes. Two knobs bound the blowup, since the number of distinct call
// stacks is exponential in call-graph depth:
//
//   LACE_CONTEXT_MAX_PER_FUNC  contexts kept per function (0 = unbounded)
//   LACE_CONTEXT_MAX_DEPTH     longest call stack retained
//
// Budget exhaustion is an explicit coverage gap. Missing paths cannot be
// treated as protected merely because the retained paths hold locks.
void CCPG::propagateContexts() {
    auto envSize = [](const char* name, size_t def) -> size_t {
        const char* v = std::getenv(name);
        if (v == nullptr || *v == '\0') return def;
        char* end = nullptr;
        unsigned long parsed = std::strtoul(v, &end, 10);
        if (end == v) return def;
        return static_cast<size_t>(parsed);
    };
    const size_t maxPerFunc = envSize("LACE_CONTEXT_MAX_PER_FUNC", 24);
    const size_t maxDepth = envSize("LACE_CONTEXT_MAX_DEPTH", 12);

    std::queue<ccpg::Function*> worklist;
    for (ccpg::Function* entry : entryFunctions) {
        if (entry == nullptr) continue;
        if (entry->getContextSet().empty()) {
            entry->addContextUnique(new Context());
        }
        worklist.push(entry);
    }
    // A fork target is a thread root too, and it starts with its own empty
    // stack: a newly created thread does NOT hold whatever locks the creating
    // thread happened to hold at the fork site, so its frames must not be
    // inherited. Propagation below only follows CALL edges, so the only way a
    // fork target gets a context is this seed.
    for (ccpg::Function* function : functions) {
        if (function == nullptr) continue;
        CCPGNode* fnode = function->getFuncNode();
        if (fnode == nullptr) continue;
        bool forkTarget = false;
        for (CCPGEdge* edge : fnode->getInEdges()) {
            if (edge->getType() == CCPGEdge::EdgeType::HB) {
                forkTarget = true;
                break;
            }
        }
        if (!forkTarget) continue;
        if (function->getContextSet().empty()) {
            function->addContextUnique(new Context());
            worklist.push(function);
        }
    }

    size_t added = 0;
    size_t capped = 0;
    size_t rounds = 0;
    const size_t maxRounds = functions.size() * 8 + 1024;

    while (!worklist.empty() && rounds < maxRounds) {
        ccpg::Function* caller = worklist.front();
        worklist.pop();
        ++rounds;
        if (caller == nullptr) continue;

        ContextSet callerCtxs = caller->getContextSet();
        if (callerCtxs.empty()) continue;

        for (CCPGNode* site : caller->getNodes()) {
            if (site == nullptr) continue;
            CCPGEdge* callEdge = hasCallEdge(site);
            if (callEdge == nullptr) continue;
            ccpg::Function* callee = getFunctionByCCPGNode(callEdge->getDst());
            if (callee == nullptr || callee == caller) continue;

            bool grew = false;
            for (Context* ctx : callerCtxs) {
                if (ctx == nullptr) continue;
                if (ctx->contains(site)) { contextsIncomplete_ = true; continue; }
                if (static_cast<size_t>(ctx->size()) >= maxDepth) { contextsIncomplete_ = true; continue; }
                if (maxPerFunc != 0 && callee->contextCount() >= maxPerFunc) {
                    contextsIncomplete_ = true;
                    ++capped;
                    break;
                }
                Context* extended = ctx->extend(site);
                if (callee->addContextUnique(extended)) {
                    ++added;
                    grew = true;
                } else {
                    delete extended;
                }
            }
            if (grew) worklist.push(callee);
        }
    }

    size_t withCtx = 0;
    for (ccpg::Function* function : functions) {
        if (function != nullptr && !function->getContextSet().empty()) ++withCtx;
    }
    std::cout << "[Context] Seeded " << entryFunctions.size()
              << " root(s); propagated " << added << " call stack(s) to "
              << withCtx << "/" << functions.size() << " functions"
              << (capped > 0 ? (" (" + std::to_string(capped) +
                                " capped at " + std::to_string(maxPerFunc) +
                                "/function)")
                             : std::string())
              << std::endl;
    if (rounds >= maxRounds) {
        contextsIncomplete_ = true;
        std::cout << "[Context] Warning: propagation stopped at the "
                  << maxRounds << "-round guard; contexts may be incomplete"
                  << std::endl;
    }
}

ccpg::Function * CCPG::createFunctionByCaller(CCPGNode * caller){
    CCPGNode * callee = findCalleeByCaller(caller);
    if(callee == nullptr){
        return nullptr;
    }
    ccpg::Function * f = createFunction(callee);
    CCPGEdge * edge = createCCPGEdge(caller, callee);
    edge->setType(CCPGEdge::EdgeType::CALL);
    this->addEdge(edge);
    handleContext(caller, f);
    return f;
}

void CCPG::labelForkPotential(){
    std::queue<ccpg::Function *> functionQueue;
    for(ccpg::Function * function : functions){
        if(function->getNodesByType(ThreadAPIUtil::TYPE::FORK).size() > 0){
            functionQueue.push(function);
        }
    }

    while (!functionQueue.empty()) {
        ccpg::Function * function = functionQueue.front();
        functionQueue.pop();

        if(function->isForkPotential()){
            continue;
        }

        function->setForkPotential(true);

        FunctionSet callers = function->getCallers();
        for(ccpg::Function * caller : callers){
            if(!caller->isForkPotential()){
                functionQueue.push(caller);
            }
        }
        
    }
}

void CCPG::labelAPI(){

    for(ccpg::Function * function : functions){
        if(function->getNodesByType(ThreadAPIUtil::TYPE::ACQUIRE).size() == 1 && function->getNodesByType(ThreadAPIUtil::TYPE::RELEASE).size() == 0){
            function->setAcquirePotential(true);
        }
        if(function->getNodesByType(ThreadAPIUtil::TYPE::RELEASE).size() == 1 && function->getNodesByType(ThreadAPIUtil::TYPE::ACQUIRE).size() == 0){
            function->setReleasePotential(true);
        }
    }
    
}


void CCPG::deleteNode(CCPGNode * node){
    for(CCPGEdge * edge : node->getInEdges()){
        edge->getSrc()->removeOutEdge(edge);
        edges.erase(edge);
    }

    for(CCPGEdge * edge : node->getOutEdges()){
        edge->getDst()->removeInEdge(edge);
        edges.erase(edge);
    }

    nodes.erase(node);
    cpgNodeToCCPGNodeMap.erase(node->getCPGNode());
}

CCPGNodeSet CCPG::getEntries(){
    CCPGNodeSet entries;
    const CPG* cpg = this->getCPG();

    auto potentialEntries = AnalysisManager::getInstance()->getPointerAnalyzer()->getPotentialEntryPoints();

    for (const auto& entryInfo : potentialEntries) {
        
        const std::string& fileName = entryInfo.fileName;
        int lineNumberFromPhasar = entryInfo.lineNumber;
        
        if (fileName == "N/A" || lineNumberFromPhasar == 0) {
            continue; // 跳过无效的入口点信息
        }

        CPGNodeSet methods = cpg->getMethodsByFileName(fileName);
        if(methods.empty()){
            methods = cpg->getMethodsByFileName(NodeLoc::extractBaseFileName(fileName));
            if(methods.empty()){
                continue;
            }
        }

        for(Node* methodNode : methods){
            if( methodNode->getLineNumber() != -1 && abs(methodNode->getLineNumber() - lineNumberFromPhasar) <= 3){
                entries.insert(createCCPGNode(methodNode));
            }
        }
    }
    return entries;
}

CCPGNode* CCPG::getMain() {
    const CPG* cpg = this->getCPG();
    auto mainInfo = AnalysisManager::getInstance()->getPointerAnalyzer()->getMainFunction();
    if (mainInfo.fileName == "N/A" || mainInfo.lineNumber == 0) {
        std::cerr << "Warning: No valid main function found." << std::endl;
        return nullptr;
    }

    const std::string& mainFuncName = mainInfo.functionName;
    std::string fileName = mainInfo.fileName;
    int lineNumber = mainInfo.lineNumber;
    std::string fileName_last = NodeLoc::extractBaseFileName(fileName);

    std::cerr << "[DEBUG getMain] Looking for: funcName=" << mainFuncName 
              << ", fileName=" << fileName << ", fileName_last=" << fileName_last 
              << ", lineNumber=" << lineNumber << std::endl;

    CPGNodeSet methods = cpg->getMethodsByFileName(fileName);
    std::cerr << "[DEBUG getMain] methods from full path: " << methods.size() << std::endl;
    if(methods.empty()){
        methods = cpg->getMethodsByFileName(fileName_last);
        std::cerr << "[DEBUG getMain] methods from filename only: " << methods.size() << std::endl;
        if(methods.empty()){
            std::cerr << "Warning: No methods found in file " << fileName << " or " << fileName_last << std::endl;
            // Layer 5 diagnostic: list closest-name methods so it is
            // obvious whether the entry exists under a different file
            // path or a slightly different name (LLVM mangling, ops_table
            // members renamed by the optimizer, etc.).
            auto suggestions = cpg->findMethodSuggestions(mainFuncName, 5);
            if (!suggestions.empty()) {
                std::cerr << "[DEBUG getMain] Closest method names in CPG:";
                for (const auto& s : suggestions) std::cerr << " " << s;
                std::cerr << std::endl;
            }
            return nullptr;
        }
    }

    std::cerr << "[DEBUG getMain] Iterating " << methods.size() << " methods" << std::endl;
    // Pre-compute the demangle/syscall name variants once so each method
    // node only does an O(K) lookup instead of O(K*M). Only used after
    // the strict equality check fails, preserving the baseline path.
    std::vector<std::string> nameCandidates = CPG::allNameCandidates(mainFuncName);
    std::unordered_set<std::string> nameCandidateSet(nameCandidates.begin(),
                                                     nameCandidates.end());
    Node* bestMatch = nullptr;
    int bestDelta = std::numeric_limits<int>::max();
    for(auto it = methods.begin(); it != methods.end(); it++){
        Node * methodNode = *it;
        std::cerr << "[DEBUG getMain] Checking method: name=" << methodNode->getName()
                  << ", line=" << methodNode->getLineNumber() << std::endl;
        if (methodNode->getName() != mainFuncName &&
            nameCandidateSet.count(methodNode->getName()) == 0) {
            continue;
        }
        if(methodNode->getLineNumber() != -1) {
            int delta = abs(methodNode->getLineNumber() - lineNumber);
            if (delta <= 3) {
                std::cerr << "[DEBUG getMain] Found main! Creating CCPGNode" << std::endl;
                return createCCPGNode(methodNode);
            }
            if (delta < bestDelta) {
                bestDelta = delta;
                bestMatch = methodNode;
            }
        } else if (bestMatch == nullptr) {
            bestMatch = methodNode;
        }
    }

    if (bestMatch != nullptr) {
        std::cerr << "[DEBUG getMain] Main line mismatch; using closest match (delta="
                  << bestDelta << "). Creating CCPGNode" << std::endl;
        return createCCPGNode(bestMatch);
    }

    std::cerr << "[DEBUG getMain] Main not found after iteration" << std::endl;
    auto suggestions = cpg->findMethodSuggestions(mainFuncName, 5);
    if (!suggestions.empty()) {
        std::cerr << "[DEBUG getMain] Closest method names in CPG:";
        for (const auto& s : suggestions) std::cerr << " " << s;
        std::cerr << std::endl;
    }
    return nullptr;
}

CCPGNode * CCPG::createCCPGNode(Node* n) {
    if(n == nullptr){
        return nullptr;
    }

    // 检查n是否已经存在CCPG中
    if(containsCPGNode(n)){
        return cpgNodeToCCPGNodeMap[n];
    }

    CCPGNode* node = new CCPGNode(n, ThreadAPIUtil::getInstance()->getType(n));
    node->setId(nodes.size() + 1);
    if(node->getType() == ThreadAPIUtil::TYPE::DUMMY 
    || node->getType() == ThreadAPIUtil::TYPE::LOOP
    || node->getType() == ThreadAPIUtil::TYPE::BRANCH
    || node->getType() == ThreadAPIUtil::TYPE::GOTO
    || node->getType() == ThreadAPIUtil::TYPE::GOTOTARGET
    || node->getType() == ThreadAPIUtil::TYPE::CONTROLSTRUCTURE
    || node->getType() == ThreadAPIUtil::TYPE::RETURN
    || node->getType() == ThreadAPIUtil::TYPE::ASSIGNMENT
    || node->getType() == ThreadAPIUtil::TYPE::HARE_PAR_FOR
    || node->getType() == ThreadAPIUtil::TYPE::GLOBAL)
    {
        node->setCallSite(false);
    }
    else{
        node->setCallSite(true);
    }
    //node->setFunction(f); 
    addNode(node);
    return node;
}

CCPGEdge* CCPG::createCCPGEdge(CCPGNode* from, CCPGNode* to) {
    if(from == nullptr || to == nullptr){
        return nullptr;
    }
    CCPGEdge* edge = new CCPGEdge(from, to);
    from->addOutEdge(edge);
    to->addInEdge(edge);
    addEdge(edge);
    return edge;
}

ccpg::Function * CCPG::createFunction(CCPGNode * funcNode) {
    ccpg::Function * function = getFunctionByCCPGNode(funcNode);
    if (function != nullptr) {
        return function;
    }
    
    function = new ccpg::Function(funcNode);
    addFunction(function);

    std::queue<CCPGNode *> nodeQueue;
    nodeQueue.push(funcNode);
    funcNode->setFunction(function);

    std::unordered_set<CCPGNode*> visitedNodes; // 防止因循环等造成重复处理
    visitedNodes.insert(funcNode);

    while (!nodeQueue.empty()) {

        CCPGNode* node = nodeQueue.front();
        nodeQueue.pop();
        function->addNode(node);

        Node* cpgNode = node->getCPGNode();
        std::unordered_set<Node*> children = findChildren(cpgNode);

        for(Node* child : children){
            CCPGNode* childNode = getCCPGNodeByCPGNode(child);
            if (!childNode) {
                childNode = createCCPGNode(child);
                childNode->setFunction(function); // 新节点也需要设置其所属函数
            }
            CCPGEdge* edge = createCCPGEdge(node, childNode);
            this->addEdge(edge);
            function->addEdge(edge);

            if (visitedNodes.find(childNode) == visitedNodes.end()) {
                visitedNodes.insert(childNode);
                nodeQueue.push(childNode);
            }
        }
    }

    const std::string& funcFileName = function->getFuncNode()->getCPGNode()->getFileName();
    for (CCPGNode* node : function->getNodes()) {
        const std::string& nodeFileName = node->getCPGNode()->getFileName();
        const std::string& effectiveFileName = nodeFileName.empty() ? funcFileName : nodeFileName;
        NodeLoc loc(effectiveFileName, node->getCPGNode()->getLineNumber(), function);
        node->setNodeLoc(loc);
        locToNodeSetMap[loc].insert(node);
        function->addNodeByLoc(loc, node);
    }

    PointerAnalysisInterface* analyzer = AnalysisManager::getInstance()->getPointerAnalyzer();
    PhasarPointerAnalysis* phasarAnalyzer = static_cast<PhasarPointerAnalysis*>(analyzer);
    if (!phasarAnalyzer) {
        std::cerr << "Error: Phasar analysis backend is not initialized." << std::endl;
        return function;
    }

    const llvm::Function* llvmFunc = AliasChecker::getInstance()->getLLVMFunction(function);
    if (llvmFunc) {
        function->setLLVMFunction(llvmFunc);
    } else {
        std::cerr << "Warning: Could not map CPG function '" << function->getFuncNode()->getCPGNode()->getName()
                  << "' to an llvm::Function." << std::endl;
        return function;
    }

    for(CCPGNode* node : function->getNodes()){
        if(node->isCallSite()){
            NodeLoc loc = node->getNodeLoc();
            std::string cpgCallName = node->getCPGNode()->getName();
            auto candidateCallInsts = phasarAnalyzer->getCallInstsByLoc(loc);
            
            for (const llvm::CallInst* candidateInst : candidateCallInsts) {
                const llvm::Function* calledFunc = candidateInst->getCalledFunction();
                if (!calledFunc) {
                    continue; 
                }

                std::string llvmCallName = LLVMAnalyzer::getInstance()->demangle(calledFunc->getName().str().c_str());
                if (cpgCallName == llvmCallName) {
                    // 找到了完美的匹配！
                    node->setLLVMCallInst(candidateInst);
                    break; 
                }
            }
        }
    }

    return function;    

}

/*std::unordered_set<Node*> CCPG::findChildren(Node* node, std::unordered_set<Node*> visited_node){
    
    if(visited_node.find(node) != visited_node.end()){
        return std::unordered_set<Node*>();
    }
    else{
        visited_node.insert(node);
    }

    const CPG * cpg = this->getCPG();
    std::unordered_set<Node*> children;

    for(Edge* edge : node->outCFGEdges){

        Node* toNode = edge->getToNode();

        if(ThreadAPIUtil::getInstance()->isCCPGNode(toNode)){
            children.insert(toNode);
        }
        else{
            std::unordered_set<Node*> temp = findChildren(edge->getToNode(), visited_node);
            children.insert(temp.begin(), temp.end());
        }
    }

    return children;
}
*/

std::unordered_set<Node*> CCPG::findChildren(Node* startNode) {
    // Check cache first
    auto cachedResult = findChildrenCache.find(startNode);
    if (cachedResult != findChildrenCache.end()) {
        return cachedResult->second;
    }

    std::unordered_set<Node*> final_children;
    std::queue<Node*> worklist;
    std::unordered_set<Node*> visited_in_this_search;
    for (Edge* edge : startNode->outCFGEdges) {
        worklist.push(edge->getToNode());
    }
    visited_in_this_search.insert(startNode);
    while (!worklist.empty()) {
        Node* currentNode = worklist.front();
        worklist.pop();

        if (visited_in_this_search.count(currentNode)) {
            continue;
        }
        visited_in_this_search.insert(currentNode);

        if (ThreadAPIUtil::getInstance()->isCCPGNode(currentNode)) {
            final_children.insert(currentNode);
        } else {
            for (Edge* edge : currentNode->outCFGEdges) {
                worklist.push(edge->getToNode());
            }
        }
    }

    // Store result in cache before returning
    findChildrenCache[startNode] = final_children;
    return final_children;
}

ccpg::Function * CCPG::getFunctionByCCPGNode(CCPGNode * node){
    int id = node->getId();
    auto it = IDToFunction.find(id);
    if (it != IDToFunction.end()) {
        return it->second;
    }
    return nullptr;
}

CCPGNode * CCPG::findCalleeByCaller(CCPGNode * caller){
    AliasChecker * aliasChecker = AliasChecker::getInstance();

    if(hasCallEdge(caller)){
        for(CCPGEdge * edge : caller->getOutEdges()){
            if(edge->getType() == CCPGEdge::EdgeType::CALL){
                return edge->getDst();
            }
        }
    }

    const llvm::CallInst* callInst = caller->getLLVMCallInst();
    if (callInst) {
        auto potentialCallees = AnalysisManager::getInstance()->getPointerAnalyzer()->getCalleesOfCallAt(callInst);

        for (const llvm::Function* llvmFunc : potentialCallees) {
            Node* methodNode = cpg->findMethodByLLVMFunction(llvmFunc);
            if (methodNode) {
                return createCCPGNode(methodNode);
            }
        }
    }

    if (caller->getCPGNode()->getName() == "<operator>.pointerCall" && caller->getCPGNode()->getCode() == "task->func(task->func_arg)") {
        Node * method = cpg->findMethod("muxer_thread");
        return createCCPGNode(method);
        //
    }

    if (caller->getType() == ThreadAPIUtil::TYPE::OBJECT_INIT){
        Node * objectInitCPGNode = caller->getCPGNode();
        Node * object = objectInitCPGNode->getArgument(1);
        if(object == nullptr){
            return nullptr;
        }
        std::string name = object->getName();
        Node * method = cpg->findMethod(name);
        return createCCPGNode(method);
    }

    else if (caller->getType() == ThreadAPIUtil::TYPE::OTHER_CALL){
        Node * callCPGNode = caller->getCPGNode();
        Node * method = cpg->findMethod(callCPGNode);
        return createCCPGNode(method);
    }
    
    return nullptr;
}

bool CCPG::existsEdge(CCPGNode * src, CCPGNode * dst) const {
    for (CCPGEdge * edge : src->getOutEdges()) {
        if (edge->getDst() == dst) {
            return true;
        }
    }
    return false;
}

std::string escapeSpecialCharacters(const std::string& input) {
    std::string result;
    for (char c : input) {
        switch (c) {
            case '\n': result += "\\n"; break;  // 换行符
            case '\t': result += "\\t"; break;  // 制表符
            case '\r': result += "\\r"; break;  // 回车符
            case '\\': result += "\\\\"; break; // 反斜杠
            case '"': result += "\\\""; break; // 双引号
            case '\'': result += "\\'"; break;  // 单引号
            case '\0': result += "\\0"; break;  // 空字符
            case '\b': result += "\\b"; break;  // 退格符
            case '\f': result += "\\f"; break;  // 换页符
            case '\v': result += "\\v"; break;  // 垂直制表符
            default: result += c; break;        // 其他字符保持不变
        }
    }
    return result;
}

void CCPG::dump(fs::path outputDir) {

    std::ofstream file(outputDir / "CCPG.dot");
    file << "digraph G {" << std::endl;
    for(CCPGNode* node : nodes) {
        std::string code;
        code = node->getCPGNode()->properties.at("CODE");
        // 如果cpg的methodNodes中包含node，则将code置为node的name
        if(node->getCPGNode()->getType() == "Method") {
            code = node->getCPGNode()->getName();
        }
        code = escapeSpecialCharacters(code);
        file << node->getId() << " [label=\"" << node->getId() << "<" + ThreadAPIUtil::getInstance()->getTypeString(node->getType()) + "> " + code;
        /*for(auto pair : node->getContextLockSet()) {
            file << "\\n";
            file << "Context: " << pair.first->toString() << "\\n";
            for(Lock* lock : pair.second) {
                if(lock == nullptr) continue;
                file << "lock" << lock->getId() << " ";
            }
        }*/
        file << "\"];" << std::endl;
        
    }
    for(CCPGEdge* edge : edges) {
        file << edge->getSrc()->getId() << " -> " << edge->getDst()->getId() << " [label=\"" << edge->getTypeString() << "\"];" << std::endl;
    }
    file << "}" << std::endl;
    file.close();

    // create dot for every function
    fs::path functionsOutputDir = outputDir / "functions";
    if (!fs::exists(functionsOutputDir)) {
        fs::create_directory(functionsOutputDir);
    }
    for(ccpg::Function* function : functions) {
        std::ofstream file(functionsOutputDir / (function->getFuncNode()->getCPGNode()->getName() + ".dot"));
        file << "digraph G {" << std::endl;
        for(CCPGNode* node : function->getNodes()) {
            std::string code;
            code = node->getCPGNode()->properties.at("CODE");
            // 如果cpg的methodNodes中包含node，则将code置为node的name
            if(node->getCPGNode()->getType() == "Method") {
                code = node->getCPGNode()->getName();
            }
            code = escapeSpecialCharacters(code);
            file << node->getId() << " [label=\"" << node->getId() << "<" + ThreadAPIUtil::getInstance()->getTypeString(node->getType()) + ">" + code;
            file << "\"];" << std::endl;
        }
        for(CCPGEdge* edge : function->getEdges()) {
            file << edge->getSrc()->getId() << " -> " << edge->getDst()->getId() << " [label=\"" << edge->getTypeString() << "\"];" << std::endl;
        }
        file << "}" << std::endl;
        file.close();
    }

    // create dot for every thread
    fs::path threadsOutputDir = outputDir / "threads";
    if (!fs::exists(threadsOutputDir)) {
        fs::create_directory(threadsOutputDir);
    }
    for(Thread* thread : ThreadCreationTree::getInstance()->getThreads()) {
        std::ofstream file(threadsOutputDir / (std::to_string(thread->getId()) + ".dot"));
        file << "digraph G {" << std::endl;
        for(CCPGNode* node : thread->getNodes()) {
            std::string code;
            code = node->getCPGNode()->properties.at("CODE");
            // 如果cpg的methodNodes中包含node，则将code置为node的name
            if(node->getCPGNode()->getType() == "Method") {
                code = node->getCPGNode()->getName();
            }
            code = escapeSpecialCharacters(code);
            file << node->getId() << " [label=\"" << node->getId() << "<" + ThreadAPIUtil::getInstance()->getTypeString(node->getType()) + ">" + code;
            file << "\"];" << std::endl;
        }
        for(CCPGEdge* edge : thread->getEdges()) {
            file << edge->getSrc()->getId() << " -> " << edge->getDst()->getId() << " [label=\"" << edge->getTypeString() << "\"];" << std::endl;
        }
        file << "}" << std::endl;
        file.close();
    }

    ThreadCreationTree::getInstance()->printThreadCreationTree(outputDir);
}