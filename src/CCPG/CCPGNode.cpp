#include "CCPG/LSAnalysis.h"

FunctionSet ccpg::Function::getCallers() const {
    FunctionSet callers;
    for(CCPGEdge * edge : funcNode->getInEdges()) {
        if(edge->getType() == CCPGEdge::EdgeType::CALL) {
            callers.insert(edge->getSrc()->getFunction());
        }
    }

    return callers;
} 

CCPGNodeSet ccpg::Function::getCallSites() const {
    CCPGNodeSet callSites;
    for(CCPGEdge * edge : funcNode->getInEdges()) {
        if(edge->getType() == CCPGEdge::EdgeType::CALL) {
            callSites.insert(edge->getSrc());
        }
    }

    return callSites;
}

int ccpg::Function::getCallOrder(const ccpg::Function * childFunc) const {
    
    for(CCPGNode * callsite : childFunc->getCallSites()){
        if(callsite->getFunction() == this){
            return callsite->getControlFlowOrder();
        }
    }

    return -1;
}