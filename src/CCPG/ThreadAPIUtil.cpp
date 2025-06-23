
#include "CCPG/ThreadAPIUtil.h"
#include "CPG/Node.h"

ThreadAPIUtil* ThreadAPIUtil::instance = nullptr;

namespace
{
/// string and type pair
struct callNameToTypePair
{
    std::string api;
    ThreadAPIUtil::TYPE t;
};

} // End anonymous namespace

//Each (name, type) pair will be inserted into the map.
//All entries of the same type must occur together (for error detection).
static callNameToTypePair callNameToTypePairs[]=
{
    //The current llvm-gcc puts in the \01.
    {"celixThread_create", ThreadAPIUtil::TYPE::FORK},
    {"pthread_create", ThreadAPIUtil::TYPE::FORK},
    {"apr_thread_create", ThreadAPIUtil::TYPE::FORK},
    {"vdbeSorterCreateThread", ThreadAPIUtil::TYPE::FORK},
    {"MtSync_Create", ThreadAPIUtil::TYPE::FORK},
    {"pthread_join", ThreadAPIUtil::TYPE::JOIN},
    {"pthread_cancel", ThreadAPIUtil::TYPE::JOIN},
    {"vdbeSorterJoinThread", ThreadAPIUtil::TYPE::JOIN},
    {"MtSync_Destruct", ThreadAPIUtil::TYPE::JOIN},
    {"pthread_mutex_lock", ThreadAPIUtil::TYPE::ACQUIRE},
    {"pthread_rwlock_rdlock", ThreadAPIUtil::TYPE::ACQUIRE},
    {"sem_wait", ThreadAPIUtil::TYPE::ACQUIRE},
    {"_spin_lock", ThreadAPIUtil::TYPE::ACQUIRE},
    {"release_", ThreadAPIUtil::TYPE::ACQUIRE},
    {"SRE_SplSpecLockEx", ThreadAPIUtil::TYPE::ACQUIRE},
    {"pthread_mutex_trylock", ThreadAPIUtil::TYPE::TRY_ACQUIRE},
    {"pthread_mutex_unlock", ThreadAPIUtil::TYPE::RELEASE},
    {"possess_", ThreadAPIUtil::TYPE::RELEASE},
    {"pthread_rwlock_unlock", ThreadAPIUtil::TYPE::RELEASE},
    {"sem_post", ThreadAPIUtil::TYPE::RELEASE},
    {"_spin_unlock", ThreadAPIUtil::TYPE::RELEASE},
    {"SRE_SplSpecUnlockEx", ThreadAPIUtil::TYPE::RELEASE},
//    {"pthread_cancel", ThreadAPIUtil::TYPE::CANCEL},
    {"pthread_exit", ThreadAPIUtil::TYPE::EXIT},
    {"pthread_detach", ThreadAPIUtil::TYPE::DETACH},
    {"pthread_cond_wait", ThreadAPIUtil::TYPE::COND_WAIT},
    {"pthread_cond_signal", ThreadAPIUtil::TYPE::COND_SIGNAL},
    {"pthread_cond_broadcast",ThreadAPIUtil::TYPE::COND_BROADCAST},
    {"pthread_cond_init", ThreadAPIUtil::TYPE::CONDVAR_INI},
    {"pthread_cond_destroy",ThreadAPIUtil::TYPE::CONDVAR_DESTROY},
    {"pthread_mutex_init", ThreadAPIUtil::TYPE::MUTEX_INI},
    {"pthread_mutex_destroy", ThreadAPIUtil::TYPE::MUTEX_DESTROY},
    {"pthread_barrier_init", ThreadAPIUtil::TYPE::BAR_INIT},
    {"pthread_barrier_wait", ThreadAPIUtil::TYPE::BAR_WAIT},

    // Hare APIs
    {"hare_parallel_for", ThreadAPIUtil::TYPE::HARE_PAR_FOR},

    //This must be the last entry.
    {"0", ThreadAPIUtil::TYPE::DUMMY}
};

void ThreadAPIUtil::init(){
    for(callNameToTypePair* p = callNameToTypePairs; p->api != "0"; p++)
    {
        threadAPIMap[p->api] = p->t;
    }
}

bool ThreadAPIUtil::isFork(Node* node)
{   
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);
    std::string callName = callNode->getName();
    if(threadAPIMap.find(callName) != threadAPIMap.end()){
        return threadAPIMap[callName] == ThreadAPIUtil::TYPE::FORK;
    }
    return false;
}

bool ThreadAPIUtil::isJoin(Node* node)
{
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);
    std::string callName = callNode->getName();
    if(threadAPIMap.find(callName) != threadAPIMap.end()){
        return threadAPIMap[callName] == ThreadAPIUtil::TYPE::JOIN;
    }
    return false;
}

bool ThreadAPIUtil::isDetach(Node* node)
{
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);
    return callNode->getName() == "pthread_detach";
}

bool ThreadAPIUtil::isAcquire(Node* node)
{
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);
    std::string callName = callNode->getName();
    if(threadAPIMap.find(callName) != threadAPIMap.end()){
        return threadAPIMap[callName] == ThreadAPIUtil::TYPE::ACQUIRE;
    }
    return false;
}

bool ThreadAPIUtil::isTryAcquire(Node* node)
{
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);
    std::string callName = callNode->getName();
    if(threadAPIMap.find(callName) != threadAPIMap.end()){
        return threadAPIMap[callName] == ThreadAPIUtil::TYPE::TRY_ACQUIRE;
    }
    return false;
}

bool ThreadAPIUtil::isRelease(Node* node)
{
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);
    std::string callName = callNode->getName();
    if(threadAPIMap.find(callName) != threadAPIMap.end()){
        return threadAPIMap[callName] == ThreadAPIUtil::TYPE::RELEASE;
    }
    return false;
}

bool ThreadAPIUtil::isExit(Node* node)
{
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);
    std::string callName = callNode->getName();
    if(threadAPIMap.find(callName) != threadAPIMap.end()){
        return threadAPIMap[callName] == ThreadAPIUtil::TYPE::EXIT;
    }
    return false;
}

bool ThreadAPIUtil::isCancel(Node* node)
{
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);
    std::string callName = callNode->getName();
    if(threadAPIMap.find(callName) != threadAPIMap.end()){
        return threadAPIMap[callName] == ThreadAPIUtil::TYPE::CANCEL;
    }
    return false;
}

bool ThreadAPIUtil::isCondWait(Node* node)
{
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);
    std::string callName = callNode->getName();
    if(threadAPIMap.find(callName) != threadAPIMap.end()){
        return threadAPIMap[callName] == ThreadAPIUtil::TYPE::COND_WAIT;
    }
    return false;
}

bool ThreadAPIUtil::isCondSignal(Node* node)
{
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);
    std::string callName = callNode->getName();
    if(threadAPIMap.find(callName) != threadAPIMap.end()){
        return threadAPIMap[callName] == ThreadAPIUtil::TYPE::COND_SIGNAL;
    }
    return false;
}

bool ThreadAPIUtil::isCondBroadcast(Node* node)
{
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);
    std::string callName = callNode->getName();
    if(threadAPIMap.find(callName) != threadAPIMap.end()){
        return threadAPIMap[callName] == ThreadAPIUtil::TYPE::COND_BROADCAST;
    }
    return false;
}


bool ThreadAPIUtil::isCondvarIni(Node* node)
{
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);
    std::string callName = callNode->getName();
    if(threadAPIMap.find(callName) != threadAPIMap.end()){
        return threadAPIMap[callName] == ThreadAPIUtil::TYPE::CONDVAR_INI;
    }
    return false;
}

bool ThreadAPIUtil::isCondvarDestroy(Node* node)
{
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);
    std::string callName = callNode->getName();
    if(threadAPIMap.find(callName) != threadAPIMap.end()){
        return threadAPIMap[callName] == ThreadAPIUtil::TYPE::CONDVAR_DESTROY;
    }
    return false;
}

bool ThreadAPIUtil::isMutexIni(Node* node)
{
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);
    std::string callName = callNode->getName();
    if(threadAPIMap.find(callName) != threadAPIMap.end()){
        return threadAPIMap[callName] == ThreadAPIUtil::TYPE::MUTEX_INI;
    }
    return false;
}

bool ThreadAPIUtil::isMutexDestroy(Node* node)
{
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);
    std::string callName = callNode->getName();
    if(threadAPIMap.find(callName) != threadAPIMap.end()){
        return threadAPIMap[callName] == ThreadAPIUtil::TYPE::MUTEX_DESTROY;
    }
    return false;
}

bool ThreadAPIUtil::isBarInit(Node* node)
{
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);
    std::string callName = callNode->getName();
    if(threadAPIMap.find(callName) != threadAPIMap.end()){
        return threadAPIMap[callName] == ThreadAPIUtil::TYPE::BAR_INIT;
    }
    return false;
}

bool ThreadAPIUtil::isBarWait(Node* node)
{
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);
    std::string callName = callNode->getName();
    if(threadAPIMap.find(callName) != threadAPIMap.end()){
        return threadAPIMap[callName] == ThreadAPIUtil::TYPE::BAR_WAIT;
    }
    return false;
}

bool ThreadAPIUtil::isAssignment(Node* node)
{
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);
    return callNode->getName().find("<operator>.assignment") != std::string::npos || callNode->getName().find("<operator>.post") != std::string::npos || callNode->getName().find("<operator>.pre") != std::string::npos;
}

bool ThreadAPIUtil::isOtherCall(Node* node)
{
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);

    if(threadAPIMap.find(callNode->getName()) == threadAPIMap.end() 
    && callNode->getName() == "<operator>.pointerCall") return true;

    return threadAPIMap.find(callNode->getName()) == threadAPIMap.end() 
    && callNode->getName().find("<operator>") == std::string::npos
    && callNode->getName().find(":") == std::string::npos;
}

bool ThreadAPIUtil::isBranch(Node* node)
{
    CallNode* callNode = static_cast<CallNode*>(node);
    return callNode->outCFGEdges.size() > 1;
}


/*bool ThreadAPIUtil::isLoop(Node* node)
{
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);
    return callNode->outCFGEdges.size() == 1;
}*/

bool ThreadAPIUtil:: isObjectInit(Node* node)
{
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);
    return callNode->getName() == "<operator>.new" ;
}

bool ThreadAPIUtil::isHareParFor(Node* node)
{
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);
    std::string callName = callNode->getName();
    if(threadAPIMap.find(callName) != threadAPIMap.end()){
        return threadAPIMap[callName] == ThreadAPIUtil::TYPE::HARE_PAR_FOR;
    }
    return false;
}

bool ThreadAPIUtil::isGoto(Node* node)
{
    return node->getType() == "Control_structure" && node->properties["CONTROL_STRUCTURE_TYPE"] == "GOTO";
}

bool ThreadAPIUtil::isGotoTarget(Node* node)
{
    return node->getType() == "Jump_target";
}

bool ThreadAPIUtil::isControlStructure(Node* node)
{
    return node->getType() == "Control_structure";
}

bool ThreadAPIUtil::isReturn(Node* node)
{
    return node->getType() == "Return";
}

bool ThreadAPIUtil::isCCPGNode(Node* node)
{
    return isFork(node) || 
           isJoin(node) || 
           isAcquire(node) || 
           isTryAcquire(node) || 
           isRelease(node) || 
           isExit(node) || 
           isCancel(node) || 
           isCondWait(node) || 
           isCondSignal(node) || 
           isCondBroadcast(node) || 
           isCondvarIni(node) || 
           isCondvarDestroy(node) || 
           isMutexIni(node) || 
           isMutexDestroy(node) || 
           isBarInit(node) || 
           isBarWait(node) || 
           isAssignment(node) || 
           isOtherCall(node) || 
           isBranch(node) || 
           isHareParFor(node) || 
           isDetach(node) || 
           isObjectInit(node) || 
           isGoto(node) || 
           isGotoTarget(node) ||
           isControlStructure(node) ||
           isReturn(node);
}


ThreadAPIUtil::TYPE ThreadAPIUtil::getType(Node* node)
{
    if(isAcquire(node)) return ThreadAPIUtil::TYPE::ACQUIRE;
    if(isAssignment(node)) return ThreadAPIUtil::TYPE::ASSIGNMENT;
    if(isBarInit(node)) return ThreadAPIUtil::TYPE::BAR_INIT;
    if(isBarWait(node)) return ThreadAPIUtil::TYPE::BAR_WAIT;
    if(isCancel(node)) return ThreadAPIUtil::TYPE::CANCEL;
    if(isCondBroadcast(node)) return ThreadAPIUtil::TYPE::COND_BROADCAST;
    if(isCondSignal(node)) return ThreadAPIUtil::TYPE::COND_SIGNAL;
    if(isCondvarDestroy(node)) return ThreadAPIUtil::TYPE::CONDVAR_DESTROY;
    if(isCondvarIni(node)) return ThreadAPIUtil::TYPE::CONDVAR_INI;
    if(isCondWait(node)) return ThreadAPIUtil::TYPE::COND_WAIT;
    if(isExit(node)) return ThreadAPIUtil::TYPE::EXIT;
    if(isFork(node)) return ThreadAPIUtil::TYPE::FORK;
    if(isJoin(node)) return ThreadAPIUtil::TYPE::JOIN;
    if(isDetach(node)) return ThreadAPIUtil::TYPE::DETACH;
    if(isMutexDestroy(node)) return ThreadAPIUtil::TYPE::MUTEX_DESTROY;
    if(isMutexIni(node)) return ThreadAPIUtil::TYPE::MUTEX_INI;
    if(isRelease(node)) return ThreadAPIUtil::TYPE::RELEASE;
    if(isTryAcquire(node)) return ThreadAPIUtil::TYPE::TRY_ACQUIRE;
    if(isHareParFor(node)) return ThreadAPIUtil::TYPE::HARE_PAR_FOR;
    if(isObjectInit(node)) return ThreadAPIUtil::TYPE::OBJECT_INIT;
    if(isGoto(node)) return ThreadAPIUtil::TYPE::GOTO;
    if(isGotoTarget(node)) return ThreadAPIUtil::TYPE::GOTOTARGET;
    if(isControlStructure(node)) return ThreadAPIUtil::TYPE::CONTROLSTRUCTURE;
    if(isOtherCall(node)) return ThreadAPIUtil::TYPE::OTHER_CALL;
    if(isBranch(node)) return ThreadAPIUtil::TYPE::BRANCH;
    
    return ThreadAPIUtil::TYPE::DUMMY;
}


/*int ThreadAPIUtil::getThreadOrLockArgument(ThreadAPIUtil::TYPE type){
    switch(type){
        case ThreadAPIUtil::TYPE::FORK:
            return 1;
        case ThreadAPIUtil::TYPE::JOIN:
            return 1;
        case ThreadAPIUtil::TYPE::CANCEL:
            return 1;
        case ThreadAPIUtil::TYPE::ACQUIRE:
            return 1;
        case ThreadAPIUtil::TYPE::TRY_ACQUIRE:
            return 1;
        case ThreadAPIUtil::TYPE::RELEASE:
            return 1;
        case ThreadAPIUtil::TYPE::EXIT:
            return 1;
        case ThreadAPIUtil::TYPE::DETACH:
            return 1;
        case ThreadAPIUtil::TYPE::COND_WAIT:
            return 2;
        case ThreadAPIUtil::TYPE::COND_SIGNAL:
            return 1;
        case ThreadAPIUtil::TYPE::COND_BROADCAST:
            return 1;
        case ThreadAPIUtil::TYPE::CONDVAR_INI:
            return 1;
        case ThreadAPIUtil::TYPE::CONDVAR_DESTROY:
            return 1;
        case ThreadAPIUtil::TYPE::MUTEX_INI:
            return 1;
        case ThreadAPIUtil::TYPE::MUTEX_DESTROY:
            return 1;
        case ThreadAPIUtil::TYPE::BAR_INIT:
            return 2;
        case ThreadAPIUtil::TYPE::BAR_WAIT:
            return 1;
        default:
            return 0;
    }
}*/



