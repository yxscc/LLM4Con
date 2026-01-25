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

#include "CCPG/HB.h"
#include "PhasarUtil/LLVMAnalyzer.h"
#include "CCPG/AliasChecker.h"
#include "CCPG/LSAnalysis.h"
#include "Util/ExecutionTimer.h"
#include "PhasarUtil/AnalysisManager.h"
#include "PhasarUtil/PhasarPointerAnalysis.h"

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

void CCPG::build(){

    const CPG* cpg = this->getCPG();
    ThreadCreationTree* tree = ThreadCreationTree::getInstance();
    tree->setCPG(cpg);
    tree->setCCPG(this);
    
    std::queue<ccpg::Function *> functionQueue;
    std::set<ccpg::Function *> visited;

    CCPGNode* main = getMain();
    if(main != nullptr){
        ccpg::Function * f = createFunction(main);
        entryFunctions.insert(f);
        functionQueue.push(f);
    }
    
    // NEW: For kernel modules without explicit main/thread creation,
    // treat all discovered entry points as potential parallel entry points
    auto pointerAnalyzer = dynamic_cast<PhasarPointerAnalysis*>(
        AnalysisManager::getInstance()->getPointerAnalyzer());
    if (pointerAnalyzer) {
        auto allEntries = pointerAnalyzer->getAllEntryPointInfos();
        if (allEntries.size() > 1) {
            std::cout << "[Kernel Module Mode] Adding " << allEntries.size() 
                      << " entry points as parallel entries" << std::endl;
            for (const auto& entryInfo : allEntries) {
                // Skip the main entry point we already added
                if (main != nullptr && main->getCPGNode()->getName() == entryInfo.functionName) {
                    std::cout << "  - Skipping (already main): " << entryInfo.functionName << std::endl;
                    continue;
                }
                
                // Demangle the function name for CPG lookup
                std::string demangledName = LLVMAnalyzer::getInstance()->demangle(entryInfo.functionName.c_str());
                
                // Extract short function name from demangled name
                // e.g., "leveldb::DBImpl::Get(leveldb::ReadOptions const&, ...)" -> "Get"
                std::string shortName = demangledName;
                
                // Remove parameters (everything after '(')
                size_t parenPos = shortName.find('(');
                if (parenPos != std::string::npos) {
                    shortName = shortName.substr(0, parenPos);
                }
                
                // Extract the last component after '::'
                size_t lastColon = shortName.rfind("::");
                if (lastColon != std::string::npos) {
                    shortName = shortName.substr(lastColon + 2);
                }
                
                // Handle destructor (remove leading '~' for lookup, will match ~ClassName)
                std::string lookupName = shortName;
                
                std::cout << "  - Looking for: " << entryInfo.functionName 
                          << " -> demangled: " << demangledName 
                          << " -> shortName: " << shortName << std::endl;
                
                // Find the method node in CPG using short name
                Node* methodNode = cpg->findMethod(shortName);
                if (methodNode == nullptr && shortName != demangledName) {
                    // Try full demangled name as fallback
                    methodNode = cpg->findMethod(demangledName);
                }
                if (methodNode == nullptr) {
                    // Try original mangled name as last resort
                    methodNode = cpg->findMethod(entryInfo.functionName);
                }
                if (methodNode == nullptr) {
                    std::cout << "  - Not found in CPG: " << shortName << " (tried: " << demangledName << ")" << std::endl;
                    continue;
                }
                
                std::cout << "  - Found in CPG: " << methodNode->getName() 
                          << " at " << methodNode->getFileName() << ":" << methodNode->getLineNumber() << std::endl;
                
                // Check if already added before creating
                if (containsCPGNode(methodNode)) {
                    std::cout << "  - Already exists: " << entryInfo.functionName << std::endl;
                    continue;
                }
                
                CCPGNode* entryNode = createCCPGNode(methodNode);
                if (entryNode != nullptr) {
                    ccpg::Function* f = createFunction(entryNode);
                    if (f != nullptr) {
                        entryFunctions.insert(f);
                        functionQueue.push(f);
                        std::cout << "  - Added entry: " << entryInfo.functionName << std::endl;
                    }
                }
            }
            std::cout << "[Kernel Module Mode] Total entry functions: " << entryFunctions.size() << std::endl;
        }
    }
    
    int i = 0;
    // create function for each call node
    while (!functionQueue.empty()) {
        ccpg::Function * function = functionQueue.front();
        functionQueue.pop();
        if(visited.count(function)){
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

    labelAPI();

    tree->handleJoins();

    ExecutionTimer::getInstance()->start("LockSet Analysis");
    LSAnalysis * lsAnalysis = LSAnalysis::getInstance();
    lsAnalysis->setCCPG(this);
    lsAnalysis->build();
    ExecutionTimer::getInstance()->stop("LockSet Analysis");
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

    auto potentialEntries = AnalysisManager::getInstance()->getPointerAnalyzer()->getPotentialEntryPoints();

    for (const auto& entryInfo : potentialEntries) {
        
        const std::string& fileName = entryInfo.fileName;
        int lineNumberFromPhasar = entryInfo.lineNumber;
        
        if (fileName == "N/A" || lineNumberFromPhasar == 0) {
            continue; // 跳过无效的入口点信息
        }

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
    std::string fileName_last = fileName.substr(fileName.find_last_of("/") + 1);

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
            return nullptr;
        }
    }

    std::cerr << "[DEBUG getMain] Iterating " << methods.size() << " methods" << std::endl;
    Node* bestMatch = nullptr;
    int bestDelta = std::numeric_limits<int>::max();
    for(auto it = methods.begin(); it != methods.end(); it++){
        Node * methodNode = *it;
        std::cerr << "[DEBUG getMain] Checking method: name=" << methodNode->getName()
                  << ", line=" << methodNode->getLineNumber() << std::endl;
        if (methodNode->getName() != mainFuncName) {
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
    return nullptr;
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