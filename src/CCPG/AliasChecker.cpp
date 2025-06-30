#include "CCPG/AliasChecker.h"
#include "SVFUtil/SVFAnalyzer.h"
#include "CCPG/ThreadCreationTree.h"

using namespace SVF;

AliasChecker * AliasChecker::instance = nullptr;


const SVF::SVFFunction* AliasChecker::getSVFFunction(ccpg::Function * function){

    if(function->getSVFFunction() != nullptr){
        return function->getSVFFunction();
    }

    SVFAnalyzer * svfAnalyzer = SVFAnalyzer::getInstance();

    for(const SVFFunction* svfFunction : SVFManager::getInstance()->getSVFModule()->getFunctionSet()){
        //std::cout << demangle(svfFunction->getName().c_str()) << std::endl;
        Node * cpgMethodNode = function->getFuncNode()->getCPGNode();
        
        if(svfAnalyzer->getLineNumberFromSourceLoc(svfFunction->getSourceLoc()) == cpgMethodNode->getLineNumber()
        && svfAnalyzer->demangle(svfFunction->getName().c_str()).find(cpgMethodNode->getName()) != std::string::npos){
            return svfFunction;
        }

        if(svfAnalyzer->demangle(svfFunction->getName().c_str()) == cpgMethodNode->getName()){
            return svfFunction;
        }
    }

    return nullptr;
    
}

Node * AliasChecker::findMethodBySVFFunction(const SVFFunction * svfFunction) const {
    const CPG * cpg = ThreadCreationTree::getInstance()->getCPG();
    SVFAnalyzer * svfAnalyzer = SVFAnalyzer::getInstance();
    CPGNodeSet methodNodes = cpg->getNodesByType("Method");
    int lineNumber = svfAnalyzer->getLineNumberFromSourceLoc(svfFunction->getSourceLoc());
    std::string fileName = svfAnalyzer->getFileFromSourceLoc(svfFunction->getSourceLoc());

    for(Node* methodNode : methodNodes){
        if(methodNode->getLineNumber() != -1 
        && abs(methodNode->getLineNumber() - lineNumber) <= 3
        && svfAnalyzer->demangle(svfFunction->getName().c_str()).find(methodNode->getName()) != std::string::npos){
            if(methodNode->properties["CODE"] != "<empty>"){
                if(methodNode->outCFGEdges.size() == 1){
                    std::unordered_set<Edge*> outEdges = methodNode->outCFGEdges;
                    Edge* edge = *outEdges.begin();
                    Node* nextNode = edge->getToNode();
                    if(nextNode->getType() == "Method_return"){
                        continue;
                    }
                }
                return methodNode;
            }
        } 
    }
    return nullptr;
}

bool AliasChecker::areCallsSame(const CallICFGNode* svfNode, Node* joernNode) {
    // 获取行号
    int svfLine = SVFAnalyzer::getInstance()->getLineNumberFromSourceLoc(svfNode->getSourceLoc());
    int joernLine = joernNode->getLineNumber();

    // 如果行号不一致，直接返回false
    if (svfLine != joernLine) return false;

    // 获取函数名
    std::string svfName = SVFAnalyzer::getInstance()->demangle(svfNode->getCalledFunction()->getName().c_str());
    std::string joernName = joernNode->getName();

    // 正则表达式提取SVF中的函数名部分
    std::regex svfPattern(R"(::(\w+<.*>)$)");
    std::smatch match;

    std::string extractedName;
    if (std::regex_search(svfName, match, svfPattern)) {
        extractedName = match[1].str();

        // 去掉模板参数中的命名空间前缀
        std::regex templatePattern(R"(<\w+::(\w+)>)");
        extractedName = std::regex_replace(extractedName, templatePattern, "<$1>");
    } else {
        // 如果没有匹配，使用原始名称
        extractedName = svfName;
    }

    // 比较名称
    return (extractedName == joernName) || extractedName.find(joernName) != std::string::npos;
}

const SVFVar * getThreadSVFValue(CCPGNode* node){
    if(node->getType() == ThreadAPIUtil::TYPE::FORK){
        return node->getCallICFGNode()->getArgument(0);
    }
    else if(node->getType() == ThreadAPIUtil::TYPE::JOIN){
        const CallICFGNode * inst = node->getCallICFGNode();
        const SVFVar* join = inst->getArgument(0);
        for(const SVFStmt* stmt : join->getInEdges())
        {
            if(const LoadStmt* l = SVFUtil::dyn_cast<LoadStmt>(stmt))
                return l->getRHSVar();
        }
        if(SVFUtil::isa<SVFArgument>(join->getValue()))
            return join;
    }
    return nullptr;
}

const SVFVar * getFreeSVFValue(const CallICFGNode* node){

    const SVFVar* free = node->getArgument(0);
    for(const SVFStmt* stmt : free->getInEdges())
    {
        if(SVFUtil::isa<LoadStmt>(stmt))
            return stmt->getSrcNode();
    }
    return free;
}


bool AliasChecker::isThreadAlias(CCPGNode * node1, CCPGNode * node2) {
    assert(node1->getType() == ThreadAPIUtil::TYPE::FORK || node1->getType() == ThreadAPIUtil::TYPE::JOIN);
    assert(node2->getType() == ThreadAPIUtil::TYPE::FORK || node2->getType() == ThreadAPIUtil::TYPE::JOIN);
    if(node1->getCallICFGNode() == nullptr || node2->getCallICFGNode() == nullptr){
        return false;
    }

    const SVF::SVFVar * thread1 = getThreadSVFValue(node1);
    const SVF::SVFVar * thread2 = getThreadSVFValue(node2);

    if(thread1 == thread2){
        return true;
    }


    if(thread1 == nullptr || thread2 == nullptr){
        return false;
    }

    bool result = SVFManager::getInstance()->getAndersen()->alias(thread1->getId(), thread2->getId());
    
    return result;
}


bool AliasChecker::isLockAlias(CCPGNode * node1, CCPGNode * node2) {
    assert(node1->getType() == ThreadAPIUtil::TYPE::ACQUIRE || node1->getType() == ThreadAPIUtil::TYPE::RELEASE);
    assert(node2->getType() == ThreadAPIUtil::TYPE::ACQUIRE || node2->getType() == ThreadAPIUtil::TYPE::RELEASE);
    if(node1->getCallICFGNode() == nullptr || node2->getCallICFGNode() == nullptr){
        return false;
    }

    const SVF::SVFVar * lock1 = node1->getCallICFGNode()->getArgument(0);
    const SVF::SVFVar * lock2 = node2->getCallICFGNode()->getArgument(0);

    if(lock1 == nullptr || lock2 == nullptr){
        return false;
    }

    bool result = SVFManager::getInstance()->getAndersen()->alias(lock1->getId(), lock2->getId());

    return result;
}

bool AliasChecker::isSharedVar(const SVFVar * var){

    SVFManager * svfManager = SVFManager::getInstance();
    SVFG * svfg = svfManager->getSVFG();
    Andersen * ander = svfManager->getAndersen();
    SVFIR * pag = svfManager->getSVFIR();

    // check if inst is global variable access
    for(const SVF::SVFGNode* globalNode : svfg->getGlobalVFGNodes()){

        const SVF::SVFValue* globalValue = globalNode->getValue();
        if(globalValue == nullptr) continue;
        NodeID globalNodeID = pag->getValueNode(globalValue);
        if(ander->alias(var->getId(), globalNodeID)){
            return true;
        }
    }

    for(Thread * thread : ThreadCreationTree::getInstance()->getThreads()){
        ccpg::Function * f = thread->getThreadMainFunction();
        const SVFFunction * svfFunction = f->getSVFFunction();
        
        if(svfFunction == nullptr || svfFunction->arg_size() == 0) 
            continue;
            
        const SVFArgument * arg = svfFunction->getArg(0);

        if(arg == nullptr) continue;

        NodeID argNodeID = pag->getValueNode(arg);
        if(ander->alias(var->getId(), argNodeID)){
            return true;
        }
    }
}

bool AliasChecker::isSharedAccess(const LoadStmt * l){
    SVFManager * svfManager = SVFManager::getInstance();
    SVFG * svfg = svfManager->getSVFG();
    Andersen * ander = svfManager->getAndersen();
    SVFIR * pag = svfManager->getSVFIR();

    // check if inst is global variable access
    for(const SVF::SVFGNode* globalNode : svfg->getGlobalVFGNodes()){

        const SVF::SVFValue* globalValue = globalNode->getValue();
        if(globalValue == nullptr) continue;
        NodeID globalNodeID = pag->getValueNode(globalValue);
        if(ander->alias(l->getRHSVarID(), globalNodeID)){
            return true;
        }
    }

    for(Thread * thread : ThreadCreationTree::getInstance()->getThreads()){
        ccpg::Function * f = thread->getThreadMainFunction();
        const SVFFunction * svfFunction = f->getSVFFunction();
        
        if(svfFunction == nullptr || svfFunction->arg_size() == 0) 
            continue;
            
        const SVFArgument * arg = svfFunction->getArg(0);

        if(arg == nullptr) continue;

        NodeID argNodeID = pag->getValueNode(arg);
        if(ander->alias(l->getRHSVarID(), argNodeID)){
            return true;
        }
    }

    return false;
    
}

bool AliasChecker::isSharedAccess(const StoreStmt * s){
    SVFManager * svfManager = SVFManager::getInstance();
    SVFG * svfg = svfManager->getSVFG();
    Andersen * ander = svfManager->getAndersen();
    SVFIR * pag = svfManager->getSVFIR();

    // check if inst is global variable access
    for(const SVF::SVFGNode* globalNode : svfg->getGlobalVFGNodes()){

        const SVF::SVFValue* globalValue = globalNode->getValue();
        if(globalValue == nullptr) continue;
        NodeID globalNodeID = pag->getValueNode(globalValue);
        if(ander->alias(s->getLHSVarID(), globalNodeID)){
            return true;
        }
    }

    for(Thread * thread : ThreadCreationTree::getInstance()->getThreads()){
        ccpg::Function * f = thread->getThreadMainFunction();
        const SVFFunction * svfFunction = f->getSVFFunction();

        if(svfFunction == nullptr || svfFunction->arg_size() == 0) 
            continue;

        const SVF::SVFArgument * arg = svfFunction->getArg(0);

        if(arg == nullptr) continue;

        NodeID argNodeID = pag->getValueNode(arg);
        if(ander->alias(s->getLHSVarID(), argNodeID)){
            return true;
        }
    }

    return false;

}

NodeID getSVFGNodeID(const SVFStmt * stmt) {
    if(const LoadStmt* l = SVFUtil::dyn_cast<LoadStmt>(stmt)){
        return l->getRHSVarID();
    }
    else if(const StoreStmt* s = SVFUtil::dyn_cast<StoreStmt>(stmt)){
        return s->getLHSVarID();
    }
    return -1;
}

std::string getSVFVarName(const SVFStmt * stmt){
    if(const LoadStmt* l = SVFUtil::dyn_cast<LoadStmt>(stmt)){
        return l->getRHSVar()->getValueName();
    }
    else if(const StoreStmt* s = SVFUtil::dyn_cast<StoreStmt>(stmt)){
        return s->getLHSVar()->getValueName();
    }
    return "";
}

bool AliasChecker::isStmtAlias(const SVFStmt * stmt1, const SVFStmt * stmt2){
    
    SVFManager * svfManager = SVFManager::getInstance();
    Andersen * ander = svfManager->getAndersen();

    // read and read
    if(SVFUtil::isa<LoadStmt>(stmt1) && SVFUtil::isa<LoadStmt>(stmt2)){
        return false;
    }
    else{
        return ander->alias(getSVFGNodeID(stmt1), getSVFGNodeID(stmt2));
    }

}

bool isSpecialVar(const std::string & name){
    return name.find("arrayidx") != std::string::npos 
    || name.find("arraydecay") != std::string::npos 
    || name == ""
    || name.find(".") != std::string::npos
    || name == "call";
}

bool AliasChecker::areSameField(const SVFStmt * stmt1, const CCPGNodeSet & nodes1, const SVFStmt * stmt2, const CCPGNodeSet & nodes2){

    //std::cout << "stmt1: " << stmt1->toString() << std::endl;
    //std::cout << "stmt2: " << stmt2->toString() << std::endl;

    std::string name1 = SVFAnalyzer::getInstance()->demangle_valueName(getSVFVarName(stmt1).c_str());
    std::string name2 = SVFAnalyzer::getInstance()->demangle_valueName(getSVFVarName(stmt2).c_str());

    if(name1 == name2){
        return true;
    }

    bool isVar1Field = false;
    bool isVar2Field = false;

    std::regex re("(_?\\d+)+$"); // 匹配末尾的数字
    name1 = std::regex_replace(name1, re, "");
    name2 = std::regex_replace(name2, re, "");

    if(isSpecialVar(name1) || isSpecialVar(name2)){
        return false;
    }

    for(CCPGNode * node1 : nodes1){
        std::string code = node1->getCPGNode()->properties.at("CODE");
        if(code.find("->" + name1) != std::string::npos){
            isVar1Field = true;
            break;
        }
        if(code.find("." + name1) != std::string::npos){
            isVar1Field = true;
            break;
        }
    }

    for(CCPGNode * node2 : nodes2){
        std::string code = node2->getCPGNode()->properties.at("CODE");
        if(code.find("->" + name2) != std::string::npos){
            isVar2Field = true;
            break;
        }
        if(code.find("." + name2) != std::string::npos){
            isVar2Field = true;
            break;
        }
    }

    if(isVar1Field && isVar2Field){
        return name1 == name2;
    }

    return true;
}

bool AliasChecker::isUseAndFreeAlias(const CallICFGNode* node, const SVFStmt * use){
    std::string sourceLoc = node->getSourceLoc();
    int lineNumber = SVFAnalyzer::getInstance()->getLineNumberFromSourceLoc(sourceLoc);
    std::string fileName = SVFAnalyzer::getInstance()->getFileFromSourceLoc(sourceLoc);

    SVFManager * svfManager = SVFManager::getInstance();
    Andersen * ander = svfManager->getAndersen();

    const SVFVar* free = getFreeSVFValue(node);
    NodeID freeID = free->getId();
    if(!isSharedVar(free)){
        return false;
    }

    NodeID useID = getSVFGNodeID(use);
    return ander->alias(useID, freeID);
}

bool AliasChecker::isFreeAndFreeAlias(const CallICFGNode* node1, const CallICFGNode* node2){
    SVFManager * svfManager = SVFManager::getInstance();
    Andersen * ander = svfManager->getAndersen();

    const SVFVar* free1 = getFreeSVFValue(node1);
    NodeID free1ID = free1->getId();

    const SVFVar* free2 = getFreeSVFValue(node2);
    NodeID free2ID = free2->getId();

    if(!isSharedVar(free1) || !isSharedVar(free2)){
        return false;
    }

    return ander->alias(free1ID, free2ID);
}
