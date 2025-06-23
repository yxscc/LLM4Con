/*#include "CCPG/HB.h"
#include <unordered_set>
#include <queue>

HB* HB::instance = nullptr;

bool HB::isForkNode(CCPGNode* node) {
    return ThreadAPIUtil::getInstance()->getTypeString(node->getType()) == "FORK";
}

bool HB::isJoinNode(CCPGNode* node) {
    return ThreadAPIUtil::getInstance()->getTypeString(node->getType()) == "JOIN";
}

bool HB::isExitNode(CCPGNode* node) {
    return ThreadAPIUtil::getInstance()->getTypeString(node->getType()) == "EXIT";
}

bool HB::isLoopBackNode(CCPGNode* node) {
    // CCPG中GOTO的目标节点
    if(ThreadAPIUtil::getInstance()->getTypeString(node->getType()) == "GOTO_TARGET") {
        return true;
    }
    CCPG* ccpg = getCCPG();
    // CCPG中loopBegin节点
    if(ccpg->isLoopBeginNode(node)) {
        return true;
    }
    // CCPG中loopBegin节点的上一个节点
    std::unordered_set<CCPGEdge*> outEdges = node->getOutEdges();
    for(CCPGEdge* e : outEdges) {
        if(e->getTypeString() == "ORDER") {
            CCPGNode* nextNode = e->getDst();
            if(ccpg->isLoopBeginNode(nextNode)) {
                return true;
            }
        }
    }
    return false;
}

bool HB::concurrentNodesEqual(std::unordered_set<CCPGNode*> nodes_1, std::unordered_set<CCPGNode*> nodes_2) {
    if(nodes_1.size() != nodes_2.size()) {
        return false;
    }
    for(CCPGNode* node : nodes_1) {
        if(nodes_2.find(node) == nodes_2.end()) {
            return false;
        }
    }
    return true;
}

CCPGNode* HB::getStartNode(CCPGNode* forkNode) {
    std::unordered_set<CCPGEdge*> outEdges = forkNode->getOutEdges();
    for(CCPGEdge* e : outEdges) {
        if(e->getTypeString() == "HB") {
            CCPGNode* startNode = e->getDst();
            return startNode;            
        }
    }
    return nullptr;
}

void HB::buildExitMap(std::unordered_set<Thread*> threads) {
    // 方法开始节点集合
    std::unordered_set<CCPGNode*> startNodes;
    for(Thread* thread : threads){
        CCPGNode* forkNode = thread->getForkNode();
        // 过滤main函数，forkNode对应线程创建语句
        if(this->isForkNode(forkNode)) {
            // 获取方法开始节点集合，避免重复列表
            CCPGNode* startNode = this->getStartNode(forkNode);
            if(startNodes.find(startNode) == startNodes.end()) {
                startNodes.insert(startNode);
            }
        }
        else {
            this->mainNodes.insert(forkNode);
        }
    }
    std::cout << startNodes.size() << std::endl;

    for(CCPGNode* node : startNodes) {
        if(node == nullptr) {
            continue;
        }
        // 对线程开始节点进行DFS，查找即线程退出节点
        std::queue<CCPGNode*> q;
        std::unordered_set<CCPGNode*> exitNodes;
        std::unordered_set<CCPGNode*> visited;
        q.push(node);
        while(!q.empty()) {
            CCPGNode* curNode = q.front();
            q.pop();
            visited.insert(curNode);
            // thread_exit()的情况
            if(this->isExitNode(curNode)) {
                exitNodes.insert(curNode);
                continue;
            }
            std::unordered_set<CCPGEdge*> outEdges = curNode->getOutEdges();
            int orderNum = 0;
            for(CCPGEdge* e : outEdges) {
                if(e->getTypeString() == "ORDER") {
                    CCPGNode* nextNode = e->getDst();
                    if(visited.find(nextNode) == visited.end()){
                        q.push(nextNode);
                    }
                    orderNum++;
                }
            }
            if(!orderNum) { //如果没有执行的子节点（调用在执行之前）
                exitNodes.insert(curNode);
            }
        }

        // 对于一个方法开始节点，找到上游的FORK类型节点，作为exitMap的key
        std::unordered_set<CCPGEdge*> inEdges = node->getInEdges();
        for(CCPGEdge* e : inEdges) {
            if(e->getTypeString() == "HB") {
                std::cout << e->getSrc()->getCPGNode()->properties.at("CODE") << std::endl;
                this->exitMap.insert({e->getSrc(), exitNodes});
            }
        }
        
    }

    // 打印验证结果
    for(auto pair : this->exitMap) {
        CCPGNode* node = pair.first;
        std::unordered_set<CCPGNode*> exitNodes = pair.second;
        int nodeId = node->getId();
        std::cout << nodeId << std::endl;
        for(CCPGNode* exitNode : exitNodes) {
           std::string code = exitNode->getCPGNode()->properties.at("CODE");
           std::cout << code << std::endl;
        }
    }
}

void HB::addHBEdge() {
    const CCPG* ccpg = this->getCCPG();
    std::unordered_set<CCPGNode*> nodes = ccpg->getNodes();
    for(CCPGNode* node : nodes) {
        // 添加线程JOIN时的HB边
        if(this->isJoinNode(node)) {
            CCPGNode* from;
            CCPGNode* to = node;
            // JOIN节点匹配线程退出节点
            for(auto pair : this->exitMap) {
                CCPGNode* forkNode = pair.first;
                // FORK和JOIN节点进行匹配，判断是否指向同一地址，即同一线程变量指针
                if(ccpg->isThreadAlias(forkNode, node)) {
                    // std::cout << "match sucessfully" << std::endl;
                    std::unordered_set<CCPGNode*> exitNodes = pair.second;
                    // 对每一个结束节点和JOIN节点建立HB边
                    for(CCPGNode* exitNode : exitNodes) {
                        from = exitNode;
                        std::unordered_set<CCPGEdge*> outEdges = from->getOutEdges();
                        int flag = 0;
                        for(CCPGEdge* e : outEdges) {
                            // 针对线程数组情况，指向同一地址，防止重复创建
                            if(e->getDst() == to) {
                                flag = 1;
                                break;
                            }
                        }
                        if(flag) {
                            continue;
                        }
                        CCPGEdge* edge = new CCPGEdge(from, to);
                        edge->setType(CCPGEdge::EdgeType::HB);
                        from->addOutEdge(edge);
                        to->addInEdge(edge);
                        CCPG->addEdge(edge);
                    }
                }
            }
            
        }
    }
}

void HB::copyThreadNode() {
    std::stack<CCPGNode*> st;
    std::unordered_set<CCPGNode*> loopNodeSet;
    std::unordered_set<CCPGNode*> visited;
    const CCPG* ccpg = this->getCCPG();
    for(CCPGNode* mainNode : this->mainNodes) {
        st.push({mainNode});
    }

    while(!st.empty()) {
        CCPGNode* currentNode = st.top();
        st.pop();
        if(visited.find(currentNode) != visited.end()) {
            continue;
        }

        if(isForkNode(currentNode) || isJoinNode(currentNode)) {
            for(CCPGNode* loopNode : loopNodeSet) {
                // if(CCPG->isLoopNode(loopNode, currentNode)) {
                if(CCPG->isCCPGChild(loopNode, currentNode)) {               
                        CCPG->copyNode(currentNode);
                }
            }
        }

        if(CCPG->isLoopBeginNode(currentNode)) {
            loopNodeSet.insert(currentNode);
        }

        std::unordered_set<CCPGEdge*> outEdges = currentNode->getOutEdges();
        for(CCPGEdge* e : outEdges) {
            CCPGNode* nextNode = e->getDst();
            // 针对for(auto..)情况
            if(currentNode == nextNode){
                if(isForkNode(currentNode) || isJoinNode(currentNode)) {
                    CCPG->eraseCCPGEdge(e);
                    CCPG->copyNode(currentNode);
                }
            }
            st.push({nextNode});
        }

        visited.insert(currentNode);
    }
}

void HB::addConcurrentInfo() {
    std::ofstream logFile("/home/conInfoGraph/hb_order.txt");
    std::stack<std::pair<CCPGNode*, Context*>> st;
    std::unordered_map<CCPGNode*, int> nodeCountMap;
    CCPG* CCPG = this->getCCPG();
    for(CCPGNode* mainNode : this->mainNodes) {
        Context* context = new Context();
        mainNode->extendConcurrentNodes(context, std::unordered_set<CCPGNode*>());
        st.push({mainNode, context});
    }

    while(!st.empty()) {
        CCPGNode* currentNode = st.top().first;
        Context* currentContext = st.top().second;
        bool isFork = false;
        bool loopExit = true;
        st.pop();

        if(nodeCountMap.find(currentNode) != nodeCountMap.end()) {
            nodeCountMap[currentNode]++;
            if(nodeCountMap[currentNode] > 3) {
                continue;
            } 
        }
        else {
            nodeCountMap[currentNode] = 1;
        }
        
        std::string code = currentNode->getNode()->properties.at("CODE");
        if(currentNode->getNode()->getType() == "Method")
            code = currentNode->getNode()->getName();
        logFile << std::to_string(currentNode->getId())<< ": " << code << std::endl;
      
        std::unordered_set<CCPGEdge*> outEdges = currentNode->getOutEdges();
        std::unordered_set<CCPGNode*> concurrentNodes = currentNode->getConcurrentNodes(currentContext);     
        // if(currentNode->getId() == 8 || currentNode->getId() == 13)
        //     logFile << currentNode->contextForkSetToString() << std::endl;

        // 对于for入口节点特殊处理，入口和出口是同一个BREACH节点，有两个下游节点，先执行循环体，再遍历出口之后，出边都是ORDER类型
        if(CCPG->isLoopBeginNode(currentNode)) {
            for(CCPGEdge* e : outEdges) {
                CCPGNode* nextNode = e->getDst();
                if(CCPG->isLoopNode(currentNode, nextNode) && !nextNode->containsHBContext(currentContext)) {
                    nextNode->extendConcurrentNodes(currentContext, concurrentNodes);
                    st.push({nextNode, currentContext});
                    loopExit = false;
                    logFile << "out1 " << nextNode->getNode()->properties.at("CODE") << std::endl;
                }
            }

            // 等循环体执行完一遍再将出口之后的节点入栈，这能保证出口在两次遍历后继承到循环体内的并发信息
            if(loopExit){ 
                for(CCPGEdge* e : outEdges) {
                    CCPGNode* nextNode = e->getDst();
                    if(!CCPG->isLoopNode(currentNode, nextNode)) {
                        if(!CCPG->isLoopBeginNode(nextNode) && nextNode->containsHBContext(currentContext) && concurrentNodesEqual(concurrentNodes, nextNode->getConcurrentNodes(currentContext))) {
                            continue;
                        }
                        // if(CCPG->isLoopBeginNode(nextNode))
                        //     logFile << "case1" <<std::endl;
                        // if(!nextNode->containsHBContext(currentContext))
                        //     logFile << "case2" <<std::endl;
                        // if(!concurrentNodesEqual(concurrentNodes, nextNode->getConcurrentNodes(currentContext)))
                        //     logFile << "case3" <<std::endl;
                        nextNode->extendConcurrentNodes(currentContext, concurrentNodes);
                        st.push({nextNode, currentContext});
                        logFile << "out2 " << nextNode->getNode()->properties.at("CODE") << std::endl;
                    }
                }
            }
            // 下一步已经入栈，不再需要之后的判断
            continue;
        }

        if(this->isForkNode(currentNode)) {
            for(CCPGEdge* e : outEdges) {
                if(e->getTypeString() == "HB") {
                    CCPGNode* nextNode = e->getDst();
                    Context* nextContext = currentContext->extend(currentNode);
                    if(!nextNode->containsHBContext(nextContext)) {
                        nextNode->extendConcurrentNodes(nextContext, concurrentNodes);
                        st.push({nextNode, nextContext});
                    }
                }
            }
            // HB边的nextNode不记录fork自己线程的记录，currentNode下游的其他节点需要记录fork该线程的记录
            isFork = true;
        }

        if(this->isJoinNode(currentNode)) {
            for(CCPGNode* forkNode : concurrentNodes) {
                if(CCPG->isThreadAlias(forkNode, currentNode)) {
                    currentNode->joinConcurrentNode(currentContext, forkNode);
                    concurrentNodes.erase(forkNode);
                    break;
                }
            }
        }

        //到达方法的退出节点，将中途经历的FORK记录还到当前上下文的上一个调用位置，保证调用位置之后的节点能有CALL子树中的FORK记录
        if(outEdges.size() == 0) {
           conInfoCallBack(currentNode, currentContext, concurrentNodes);
        }

        // 将未访问的邻居节点压入栈，先执行CALL再执行ORDER，入栈方向相反
        for(CCPGEdge* e : outEdges) {
            if(e->getTypeString() == "ORDER") {
                CCPGNode* nextNode = e->getDst();
                // 防止重复遍历
                if(!isLoopBackNode(nextNode) && nextNode->containsHBContext(currentContext) && concurrentNodesEqual(concurrentNodes, nextNode->getConcurrentNodes(currentContext))) {
                    continue;
                }
                nextNode->extendConcurrentNodes(currentContext, concurrentNodes);
                // 子节点需要增加上一个FORK的记录
                if(isFork) {
                    nextNode->addConcurrentNode(currentContext, currentNode);
                }
                st.push({nextNode, currentContext});
                logFile << "out3" << std::endl;
            }
        }

        for(CCPGEdge* e : outEdges) {
            if(e->getTypeString() == "CALL") {
                CCPGNode* nextNode = e->getDst();
                Context* nextContext = currentContext->extend(currentNode);
                if(!nextNode->containsHBContext(nextContext)){         
                    nextNode->extendConcurrentNodes(nextContext, concurrentNodes);
                    // 子节点需要增加上一个FORK的记录
                    if(isFork) {
                        nextNode->addConcurrentNode(nextContext, currentNode);
                    }
                    st.push({nextNode, nextContext});
                }
            }
        }
    }
}

void HB::conInfoCallBack(CCPGNode* node, Context* context, std::unordered_set<CCPGNode*> concurrentNodes) {   
    // std::ofstream logFile("/home/conInfoGraph/hb_callback.txt", std::ios::app);
    // logFile << "node_id:" << std::to_string(node->getId()) << std::endl;
    // logFile << node->contextForkSetToString() << std::endl;
    std::vector<CCPGNode*> callStack = context->getCallStack();
    if(callStack.size() == 0){
        return ;
    }
    //最后一个调用位置
    CCPGNode* callNode = callStack.back();
    // logFile << "callnode_id:" << std::to_string(callNode->getId()) << std::endl;
    Context* preContext = context->popNode();
    
    std::unordered_set<CCPGEdge*> outEdges = callNode->getOutEdges();
    int orderNum = 0;
    for(CCPGEdge* e : outEdges) {
        if(e->getTypeString() == "ORDER") {
            CCPGNode* nextNode = e->getDst();
            // logFile << "nextnode_id:" << std::to_string(nextNode->getId()) << std::endl;
            orderNum++;
            Context* realContext = nextNode->findContextObj(preContext);
            if(realContext == nullptr) {
                // logFile << "fail \n" << std::endl;
                continue;
            }
            else{
                nextNode->extendConcurrentNodes(realContext, concurrentNodes); 
                // logFile << nextNode->contextForkSetToString() << std::endl;
                // logFile << "success \n" << std::endl;
            }
        }
    }

    //如果调用位置也没有ORDER边，即没有继续执行的内容，递归传递信息回上一级调用位置
    if(orderNum == 0) {
        conInfoCallBack(callNode, preContext, concurrentNodes);
    }
}


void HB::buildHB(std::unordered_set<Thread*> threads) {
    this->buildExitMap(threads);
    // this->addHBEdge();
    // std::cout << "HB edge build done" << std::endl;
    // this->copyThreadNode();
    // std::cout << "thread nodes copy done" << std::endl;
    this->addConcurrentInfo();
    this->hasHBEdge = true;
    std::cout << "build HB done!" << std::endl;
}


bool HB::concurrentMatch(Context* context, std::unordered_set<CCPGNode*> forkSet) {
    std::vector<CCPGNode*> callStack = context->getCallStack();
    // 匹配并行发生的fork节点
    for(CCPGNode* callNode : callStack) {
        if(forkSet.find(callNode) != forkSet.end()) {
            return true;
        }
    }
    return false;
}

bool HB::contextHappensBefore(CCPGNode* node1, Context* context_1, CCPGNode* node2, Context* context_2) {
    std::unordered_set<CCPGNode*> forkSet_1 = node1->getConcurrentNodes(context_1);
    std::unordered_set<CCPGNode*> forkSet_2 = node2->getConcurrentNodes(context_2);
    //std::cout << "node_id: " + std::to_string(node1->getId()) + "-" + std::to_string(node2->getId()) << std::endl;
    if(concurrentMatch(context_1, forkSet_2) || concurrentMatch(context_2, forkSet_1)) {   
        /*std::cout << "concurrent" << std::endl;
        std::cout << "node1 context:" << std::endl;
        std::cout << context_1->toString() << std::endl;
        std::cout << "node2 context:" << std::endl;
        std::cout << context_2->toString() << std::endl;
        std::cout << std::endl;*/
        // 能并行，即执行顺序不确定
        return false;
    }
    else {
        /*std::cout << "happens-before" << std::endl;
        std::cout << "node1 context:" << std::endl;
        std::cout << context_1->toString() << std::endl;
        std::cout << "node2 context:" << std::endl;
        std::cout << context_2->toString() << std::endl;
        std::cout << std::endl;*/
        // 不能并行，即执行顺序确定
        return true;
    }
}

void HB::nodeHappensBefore(CCPGNode* node1, CCPGNode* node2) {
    // if(this->hasHBEdge == false) {
    //     this->buildHB();
    // }
    CCPG* CCPG = this->getCCPG();
    std::unordered_map<Context*, std::unordered_set<CCPGNode*>> contextForkSet_1 = node1->getContextForkSet();
    std::unordered_map<Context*, std::unordered_set<CCPGNode*>> contextForkSet_2 = node2->getContextForkSet();

    //std::cout << "node_id: " + std::to_string(node1->getId()) + "-" + std::to_string(node2->getId()) << std::endl;

    int num = 1;
    
    for(auto it = contextForkSet_1.begin(); it != contextForkSet_1.end(); ++it) {
        Context* context_1 = it->first;
        std::unordered_set<CCPGNode*> forkSet_1 = it->second;
        for(auto i = contextForkSet_2.begin(); i != contextForkSet_2.end(); ++i) {
            Context* context_2 = i->first;
            std::unordered_set<CCPGNode*> forkSet_2 = i->second;
            
            if(concurrentMatch(context_1, forkSet_2) || concurrentMatch(context_2, forkSet_1)) {
                std::cout << "case" + std::to_string(num) + ":concurrent" << std::endl;
                std::cout << "node1 context:" << std::endl;
                std::cout << context_1->toString() << std::endl;
                std::cout << "node2 context:" << std::endl;
                std::cout << context_2->toString() << std::endl;
                std::cout << std::endl;
                
            }

            else {
                std::cout << "case" + std::to_string(num) + ":happens-before" << std::endl;
                std::cout << "node1 context:" << std::endl;
                std::cout << context_1->toString() << std::endl;
                std::cout << "node2 context:" << std::endl;
                std::cout << context_2->toString() << std::endl;
                std::cout << std::endl;
            }

            num += 1;
        }
    }
}