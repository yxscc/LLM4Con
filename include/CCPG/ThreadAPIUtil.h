#pragma once

#include <unordered_map>

#include "CPG/Node.h"

class ThreadAPIUtil
{
    
public:
    enum TYPE
        {
            DUMMY = 0,   /// dummy type
            FORK,        /// create a new thread
            JOIN,        /// wait for a thread to join
            DETACH,      /// detach a thread directly instead wait for it to join
            ACQUIRE,     /// acquire a lock
            TRY_ACQUIRE, /// try to acquire a lock
            RELEASE,     /// release a lock
            EXIT,        /// exit/kill a thread
            CANCEL,      /// cancel a thread by another
            COND_WAIT,   /// wait a condition
            COND_SIGNAL, /// signal a condition
            COND_BROADCAST,  /// broadcast a condition
            MUTEX_INI,       /// initial a mutex variable
            MUTEX_DESTROY,   /// destroy a mutex variable
            CONDVAR_INI,     /// initial a cond mutex variable
            CONDVAR_DESTROY, /// destroy a cond mutex variable
            BAR_INIT,        /// Barrier init
            BAR_WAIT,        /// Barrier wait
            ASSIGNMENT,      /// assignment
            OTHER_CALL,      /// other calls
            BRANCH,          /// branch
            LOOP,            /// loop
            OBJECT_INIT,     /// object initialization
            GOTO,            /// goto
            GOTOTARGET,      /// goto target
            CONTROLSTRUCTURE, /// control structure
            RETURN,           /// return
            GLOBAL,           /// global variable
            HARE_PAR_FOR
        };


    bool isFork(Node* node);
    bool isJoin(Node* node);
    bool isDetach(Node* node);
    bool isAcquire(Node* node);
    bool isTryAcquire(Node* node);
    bool isRelease(Node* node);
    bool isExit(Node* node);
    bool isCancel(Node* node);
    bool isCondWait(Node* node);
    bool isCondSignal(Node* node);
    bool isCondBroadcast(Node* node);
    bool isMutexIni(Node* node);
    bool isMutexDestroy(Node* node);
    bool isCondvarIni(Node* node);
    bool isCondvarDestroy(Node* node);
    bool isBarInit(Node* node);
    bool isBarWait(Node* node);
    bool isAssignment(Node* node);
    bool isOtherCall(Node* node);
    bool isBranch(Node* node);
    bool isLoop(Node* node);
    bool isHareParFor(Node* node);
    bool isObjectInit(Node* node);
    bool isGoto(Node* node);
    bool isGotoTarget(Node* node);
    bool isControlStructure(Node* node);
    bool isReturn(Node* node);

    bool isCCPGNode(Node* node);


    std::unordered_map<std::string, TYPE> getThreadAPIMap(){
        return threadAPIMap;
    }

    static std::string getTypeString(ThreadAPIUtil::TYPE type)
    {
        switch(type) {
            case DUMMY: return "DUMMY";
            case FORK: return "FORK";
            case JOIN: return "JOIN";
            case DETACH: return "DETACH";
            case ACQUIRE: return "ACQUIRE";
            case TRY_ACQUIRE: return "TRY_ACQUIRE";
            case RELEASE: return "RELEASE";
            case EXIT: return "EXIT";
            case CANCEL: return "CANCEL";
            case COND_WAIT: return "COND_WAIT";
            case COND_SIGNAL: return "COND_SIGNAL";
            case COND_BROADCAST: return "COND_BROADCAST";
            case MUTEX_INI: return "MUTEX_INI";
            case MUTEX_DESTROY: return "MUTEX_DESTROY";
            case CONDVAR_INI: return "CONDVAR_INI";
            case CONDVAR_DESTROY: return "CONDVAR_DESTROY";
            case BAR_INIT: return "BAR_INIT";
            case BAR_WAIT: return "BAR_WAIT";
            case ASSIGNMENT: return "ASSIGNMENT";
            case OTHER_CALL: return "OTHER_CALL";
            case BRANCH: return "BRANCH";
            case LOOP: return "LOOP";
            case HARE_PAR_FOR: return "HARE_PAR_FOR";
            case OBJECT_INIT: return "OBJECT_INIT";
            case GOTO: return "GOTO";
            case GOTOTARGET: return "GOTO_TARGET";
            case CONTROLSTRUCTURE: return "CONTROLSTRUCTURE";
            case RETURN: return "RETURN";
            default: return "Unknown Type";
        }
    }
    
    TYPE getType(Node* node);

    static ThreadAPIUtil* getInstance() {
        if (instance == nullptr) {
            instance = new ThreadAPIUtil();
        }
        return instance;
    }

    int getThreadOrLockArgument(ThreadAPIUtil::TYPE type);

private:
    std::unordered_map<std::string, TYPE> threadAPIMap;

    static ThreadAPIUtil* instance;  // 静态实例指针

    ThreadAPIUtil() { init(); }  // 私有构造函数
    ~ThreadAPIUtil() {}  // 私有析构函数
    ThreadAPIUtil(const ThreadAPIUtil&) = delete;            // 阻止复制构造
    ThreadAPIUtil& operator=(const ThreadAPIUtil&) = delete; // 阻止赋值操作
    void init();
};
