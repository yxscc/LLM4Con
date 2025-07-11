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

#include "CCPG/HB.h"
#include "SVFUtil/SVFAnalyzer.h"
#include "CCPG/AliasChecker.h"
#include "CCPG/LSAnalysis.h"
#include "Util/ExecutionTimer.h"
#include "SABER/SaberCheckerAPI.h"
#include "SVFUtil/SVFManager.h"
#include "Graphs/PTACallGraph.h"

using namespace ccpg;

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

std::unordered_map<NodeLoc, std::vector<const CallICFGNode*>, NodeLocHash> svfCallSitesByLoc;

void CCPG::build(){

    //mapSVFInstructions();
    SVFManager* svfManager = SVFManager::getInstance();
    SVFG* svfg = svfManager->getSVFG();
    SVFIR* pag = svfManager->getSVFIR();
    const CPG* cpg = this->getCPG();
    ThreadCreationTree* tree = ThreadCreationTree::getInstance();
    tree->setCPG(cpg);
    tree->setCCPG(this);

    // 遍历整个程序的所有SVF调用点，构建索引
    for (const CallICFGNode* callNode : pag->getCallSiteSet()) {
        if (callNode && callNode->getCalledFunction()) {
            std::string sourceLoc = callNode->getSourceLoc();
            int lineNumber = SVFAnalyzer::getInstance()->getLineNumberFromSourceLoc(sourceLoc);
            std::string fileName = SVFAnalyzer::getInstance()->getFileFromSourceLoc(sourceLoc);
            
            NodeLoc loc(fileName, lineNumber, nullptr); 

            // 将 callNode 指针添加到对应位置的 vector 中
            svfCallSitesByLoc[loc].push_back(callNode);
        }
    }

    std::queue<ccpg::Function *> functionQueue;
    std::set<ccpg::Function *> visited;
    CCPGNodeSet entryNodes = getEntries();
    for(CCPGNode * entry : entryNodes){
        ccpg::Function * f = createFunction(entry);
        entryFunctions.insert(f);
        functionQueue.push(f);
    }

    int i = 0;
    // create function for each call node
    while (!functionQueue.empty()) {
        ccpg::Function * function = functionQueue.front();
        functionQueue.pop();
        if(visited.find(function) != visited.end()){
            continue;
        }
        visited.insert(function);

        for(CCPGNode * objectInitNode : function->getNodesByType(ThreadAPIUtil::TYPE::OBJECT_INIT)){
            if (hasCallEdge(objectInitNode)) {
                continue;
            }
            ccpg::Function * f = createFunctionByCaller(objectInitNode);
            if(f == nullptr){
                continue;
            }
            functionQueue.push(f);
        }

        for(CCPGNode * callNode : function->getNodesByType(ThreadAPIUtil::TYPE::OTHER_CALL)){
            if (hasCallEdge(callNode)) {
                continue;
            }
            ccpg::Function * f = createFunctionByCaller(callNode);
            if(f == nullptr){
                continue;
            }
            functionQueue.push(f);
        }

        for(CCPGNode * forkNode : function->getNodesByType(ThreadAPIUtil::TYPE::FORK)){
            Node * functionCPGNode = ThreadCreationTree::getInstance()->findThreadEntryInCPG(forkNode);
            if(functionCPGNode == nullptr){
                continue;
            }
            CCPGNode * functionNode = createCCPGNode(functionCPGNode);
            ccpg::Function * f = createFunction(functionNode);
            CCPGEdge * edge = createCCPGEdge(forkNode, functionNode);
            edge->setType(CCPGEdge::EdgeType::HB);
            this->addEdge(edge);
            functionQueue.push(f);
            handleContext(forkNode, f);
        }
    }
    labelForkPotential();

    ExecutionTimer::getInstance()->start("Building thread creation tree");
    tree->build();
    ExecutionTimer::getInstance()->stop("Building thread creation tree");

    ExecutionTimer::getInstance()->start("Mapping SVF instructions");
    mapSVFInstructions();
    ExecutionTimer::getInstance()->stop("Mapping SVF instructions");

    labelAPI();

    tree->handleJoins();
    tree->countParallelThreadPairs();
    ExecutionTimer::getInstance()->stop("CCPG Analysis");
    std::unordered_map<std::pair<Thread*, Thread*>, std::string, pair_hash> parallelThreadPairs = tree->getParallelThreadPairs();
    for(auto it = parallelThreadPairs.begin(); it != parallelThreadPairs.end(); it++){
        std::pair<Thread*, Thread*> pair = it->first;
        Thread * t1 = pair.first;
        Thread * t2 = pair.second;
        std::string relation = it->second;

        std::unordered_map<NodeLoc, Context, NodeLocHash> parallelLocs1;
        std::unordered_map<NodeLoc, Context, NodeLocHash> parallelLocs2;

        // 获取两个线程的并行位置
        std::tie(parallelLocs1, parallelLocs2) = tree->getParallelLocs(t1, t2);
    }

    ExecutionTimer::getInstance()->start("LockSet Analysis");
    LSAnalysis * lsAnalysis = LSAnalysis::getInstance();
    lsAnalysis->setCCPG(this);
    lsAnalysis->build();
    ExecutionTimer::getInstance()->stop("LockSet Analysis");

    //inferTemporality();

    /*HB* hb = HB::getInstance();
    hb->setCCPG(tree->getCCPG());
    hb->buildHB(tree->getThreads());
    std::cout << "complete building happens-before relation" << std::endl;*/
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

    for(const SVFFunction * svfFunction : SVFManager::getInstance()->getSVFModule()->getFunctionSet()){

        PTACallGraph * callGraph = SVFManager::getInstance()->getPTACallGraph();

        if (SVFUtil::isExtCall(svfFunction))
            continue;

        PTACallGraphNode* node = callGraph->getCallGraphNode(svfFunction);

        if (node->hasIncomingEdge())
        {
            continue;
        }

        std::string sourceLoc = svfFunction->getSourceLoc();
        int lineNumber = SVFAnalyzer::getInstance()->getLineNumberFromSourceLoc(sourceLoc);
        std::string fileName = SVFAnalyzer::getInstance()->getFileFromSourceLoc(sourceLoc);
        std::string fileName_last = fileName.substr(fileName.find_last_of("/") + 1);


        CPGNodeSet methods = cpg->getMethodsByFileName(fileName);
        if(methods.size() == 0){
            methods = cpg->getMethodsByFileName(fileName_last);
            if(methods.size() == 0){
                continue;
            }
        }
        for(auto it = methods.begin(); it != methods.end(); it++){
            Node * methodNode = *it;
            if( methodNode->getLineNumber() != -1 && abs(methodNode->getLineNumber() - lineNumber) <= 3){ //因为有一些函数的定��是分行的，所以需要将比较条件设置宽松一点
                entries.insert(createCCPGNode(methodNode));
                
            }
        }
    }

    return entries;
}


CCPGNode * CCPG::createCCPGNode(Node* n) {
    if(n == nullptr){
        return nullptr;
    }

    if(n->getCode() == "ret = task->func(task->func_arg)"){
        int i = 1;
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
    SVFAnalyzer * svfAnalyzer = SVFAnalyzer::getInstance();
    ccpg::Function * function = getFunctionByCCPGNode(funcNode);
    if (function != nullptr) {
        return function;
    }

    funcNode->setControlFlowOrder(0);
    
    function = new ccpg::Function(funcNode);
    addFunction(function);

    std::queue<CCPGNode *> nodeQueue;
    nodeQueue.push(funcNode);

    while (!nodeQueue.empty()) {

        CCPGNode* node = nodeQueue.front();
        function->addNode(node);
        nodeQueue.pop();

        Node* cpgNode = node->getCPGNode();
        std::unordered_set<Node*> children = findChildren(cpgNode);

        for(Node* child : children){
            CCPGNode* childNode = getCCPGNodeByCPGNode(child);
            CCPGEdge* edge;
            if(childNode != nullptr ){
                edge = createCCPGEdge(node, childNode);
            }
            else{
                childNode = createCCPGNode(child);
                childNode->setFunction(function);
                edge = createCCPGEdge(node, childNode);
                nodeQueue.push(childNode);
            }
            if(childNode->getControlFlowOrder() == 0){
                childNode->setControlFlowOrder(node->getControlFlowOrder() + 1);
            }
            this->addEdge(edge);
            function->addEdge(edge);
        }
    }

    for(CCPGNode* node : function->getNodes()){
        if(node->getCPGNode()->getFileName() != ""){
            NodeLoc loc(node->getCPGNode()->getFileName(), node->getCPGNode()->getLineNumber(), function);
            node->setNodeLoc(loc);
            addNodeByLoc(loc, node);
            function->addNodeByLoc(loc, node);
            locToNodeSetMap[loc].insert(node);
        }
        else{
            NodeLoc loc(function->getFuncNode()->getCPGNode()->getFileName(), node->getCPGNode()->getLineNumber(), function);
            node->setNodeLoc(loc);
            addNodeByLoc(loc, node);
            function->addNodeByLoc(loc, node);
            locToNodeSetMap[loc].insert(node);
        }
    }

    if(function->getFuncNode()->getId() == 9){
        int i=1;
    }

    const SVF::SVFFunction* svfFunction = AliasChecker::getInstance()->getSVFFunction(function);
    function->setSVFFunction(svfFunction);
    funcNode->setFunction(function);

    if(svfFunction == nullptr){
        return function;
    }

    for(CCPGNode* node : function->getNodes()){
        if(node->isCallSite()){
            NodeLoc loc = node->getNodeLoc();
            NodeLoc lookupKey(loc.getFileName(), loc.getLineNumber(), nullptr);
            auto it = svfCallSitesByLoc.find(lookupKey);
            if (it != svfCallSitesByLoc.end()) {
                const auto& candidates = it->second;
                // 遍历这个位置的所有候选SVF调用点
                for (const CallICFGNode* candidateCallNode : candidates) {
                    // 使用 areCallsSame 进行精确匹配
                    if(AliasChecker::getInstance()->areCallsSame(candidateCallNode, node->getCPGNode())){
                        node->setCallICFGNode(candidateCallNode);
                        if (SaberCheckerAPI::getCheckerAPI()->isMemDealloc(candidateCallNode)){
                            addSpecialCall(loc, SpecialCallType::Free, candidateCallNode);
                        }
                        else if (SaberCheckerAPI::getCheckerAPI()->isMemAlloc(candidateCallNode)){
                            addSpecialCall(loc, SpecialCallType::Alloc, candidateCallNode);
                        }
                        break;
                    }
                    
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

    const SVF::CallICFGNode * callICFGNode = caller->getCallICFGNode();
    if(callICFGNode != nullptr){
        const SVFFunction * svfFunction = callICFGNode->getCalledFunction();
        Node * method = aliasChecker->findMethodBySVFFunction(svfFunction);
        if(method != nullptr){
            return createCCPGNode(method);
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



void CCPG::mapSVFInstructions(){
    AliasChecker * aliasChecker = AliasChecker::getInstance();

    SVFManager * svfManager = SVFManager::getInstance();
    SVFIR* pag = svfManager->getSVFIR();

    for(auto it = functions.begin(); it != functions.end(); it++){
        ccpg::Function * function = *it;
        const SVF::SVFFunction* svfFunction = function->getSVFFunction();

        if(svfFunction == nullptr){
            continue;
        }

        for(const SVFBasicBlock * svfbb : svfFunction->getBasicBlockList()){
            for (const ICFGNode* icfgNode : svfbb->getICFGNodeList())
            {
                for(const SVFStmt* stmt : pag->getSVFStmtList(icfgNode))
                {
                    if (const LoadStmt* l = SVFUtil::dyn_cast<LoadStmt>(stmt))
                    {
                        
                        if(l == nullptr ){ //|| !AliasChecker::getInstance()->isSharedAccess(l)
                            continue;
                        }
                        const SVFInstruction* inst = l->getInst();
                        std::string sourceLoc = inst->getSourceLoc();
                        int lineNumber = SVFAnalyzer::getInstance()->getLineNumberFromSourceLoc(sourceLoc);
                        std::string fileName = SVFAnalyzer::getInstance()->getFileFromSourceLoc(sourceLoc);
                        addSVFInstByLoc(NodeLoc(fileName, lineNumber, function), stmt);
                        addStructFieldStmt(stmt);
                    }
                    else if (const StoreStmt* s = SVFUtil::dyn_cast<StoreStmt>(stmt))
                    {
                        if(s == nullptr ){ //|| !AliasChecker::getInstance()->isSharedAccess(s)
                            continue;
                        }
                        const SVFInstruction* inst = s->getInst();
                        std::string sourceLoc = inst->getSourceLoc();
                        int lineNumber = SVFAnalyzer::getInstance()->getLineNumberFromSourceLoc(sourceLoc);
                        std::string fileName = SVFAnalyzer::getInstance()->getFileFromSourceLoc(sourceLoc);
                        addSVFInstByLoc(NodeLoc(fileName, lineNumber, function), s);
                    }
                }
            }
        }

    }
}

void CCPG::addStructFieldStmt(const SVFStmt* stmt){
    if(const LoadStmt* l = SVFUtil::dyn_cast<LoadStmt>(stmt)){
        const SVFVar * var = l->getLHSVar();
        if(var->hasOutgoingEdges(SVFStmt::Gep)){
            for(auto it = var->getOutgoingEdgesBegin(SVFStmt::Gep); it != var->getOutgoingEdgesEnd(SVFStmt::Gep); it++){
                const SVFStmt * gep = *it;
                const GepStmt * gepStmt = SVFUtil::dyn_cast<GepStmt>(gep);
                const SVFVar * gepVar = gepStmt->getLHSVar();
                if(gepVar->hasOutgoingEdges(SVFStmt::Load)){
                    for(auto it = gepVar->getOutgoingEdgesBegin(SVFStmt::Load); it != gepVar->getOutgoingEdgesEnd(SVFStmt::Load); it++){
                        const SVFStmt * load = *it;
                        const LoadStmt * loadStmt = SVFUtil::dyn_cast<LoadStmt>(load);
                        const SVFInstruction* inst = loadStmt->getInst();
                        std::string sourceLoc = inst->getSourceLoc();
                        int lineNumber = SVFAnalyzer::getInstance()->getLineNumberFromSourceLoc(sourceLoc);
                        std::string fileName = SVFAnalyzer::getInstance()->getFileFromSourceLoc(sourceLoc);
                        addSVFInstByLoc(NodeLoc(fileName, lineNumber, nullptr), loadStmt);
                        addStructFieldStmt(loadStmt);
                    }
                }
                if(gepVar->hasIncomingEdges(SVFStmt::Store)){
                    for(auto it = gepVar->getIncomingEdgesBegin(SVFStmt::Store); it != gepVar->getIncomingEdgesEnd(SVFStmt::Store); it++){
                        const SVFStmt * store = *it;
                        const StoreStmt * storeStmt = SVFUtil::dyn_cast<StoreStmt>(store);
                        const SVFInstruction* inst = storeStmt->getInst();
                        std::string sourceLoc = inst->getSourceLoc();
                        int lineNumber = SVFAnalyzer::getInstance()->getLineNumberFromSourceLoc(sourceLoc);
                        std::string fileName = SVFAnalyzer::getInstance()->getFileFromSourceLoc(sourceLoc);
                        addSVFInstByLoc(NodeLoc(fileName, lineNumber, nullptr), storeStmt);
                    }
                }
            }
        }
    }
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