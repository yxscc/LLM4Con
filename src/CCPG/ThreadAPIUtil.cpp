
#include "CCPG/ThreadAPIUtil.h"
#include "CPG/Node.h"
#include <vector>
#include <algorithm>
#include <cstring>

namespace {

bool endsWith(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() &&
           s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool isLockWrapperByName(const std::string& name) {
    static const char* suffixes[] = {
        "_lock", "_lock_irq", "_lock_irqsave", "_lock_bh",
        "_lock_nested", "_rlock", "_wlock",
    };
    for (const char* sfx : suffixes) {
        if (endsWith(name, sfx) && name.size() > std::strlen(sfx)) {
            if (name == "clock" || name == "flock") continue;
            return true;
        }
    }
    return false;
}

bool isUnlockWrapperByName(const std::string& name) {
    static const char* suffixes[] = {
        "_unlock", "_unlock_irq", "_unlock_irqrestore", "_unlock_bh",
        "_runlock", "_wunlock",
    };
    for (const char* sfx : suffixes) {
        if (endsWith(name, sfx) && name.size() > std::strlen(sfx))
            return true;
    }
    return false;
}

bool isTryLockWrapperByName(const std::string& name) {
    static const char* suffixes[] = {
        "_trylock", "_trylock_irq", "_trylock_irqsave", "_trylock_bh",
    };
    for (const char* sfx : suffixes) {
        if (endsWith(name, sfx) && name.size() > std::strlen(sfx))
            return true;
    }
    return false;
}

} // anonymous namespace

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
    //==========================================================================
    // POSIX Thread APIs
    //==========================================================================
    {"celixThread_create", ThreadAPIUtil::TYPE::FORK},
    {"pthread_create", ThreadAPIUtil::TYPE::FORK},
    {"apr_thread_create", ThreadAPIUtil::TYPE::FORK},
    {"vdbeSorterCreateThread", ThreadAPIUtil::TYPE::FORK},
    {"MtSync_Create", ThreadAPIUtil::TYPE::FORK},
    {"StartThread", ThreadAPIUtil::TYPE::FORK},  // LevelDB/common pattern
    {"Schedule", ThreadAPIUtil::TYPE::FORK},      // Common thread pool pattern
    
    //==========================================================================
    // Linux Kernel Thread Creation APIs
    //==========================================================================
    // Kernel threads (kthread)
    {"kthread_create", ThreadAPIUtil::TYPE::FORK},
    {"kthread_create_on_node", ThreadAPIUtil::TYPE::FORK},
    {"kthread_create_on_cpu", ThreadAPIUtil::TYPE::FORK},
    {"kthread_run", ThreadAPIUtil::TYPE::FORK},
    {"kthread_create_worker", ThreadAPIUtil::TYPE::FORK},
    {"kthread_create_worker_on_cpu", ThreadAPIUtil::TYPE::FORK},
    
    // Workqueue (delayed execution context)
    {"queue_work", ThreadAPIUtil::TYPE::FORK},
    {"queue_work_on", ThreadAPIUtil::TYPE::FORK},
    {"queue_delayed_work", ThreadAPIUtil::TYPE::FORK},
    {"queue_delayed_work_on", ThreadAPIUtil::TYPE::FORK},
    {"schedule_work", ThreadAPIUtil::TYPE::FORK},
    {"schedule_delayed_work", ThreadAPIUtil::TYPE::FORK},
    {"mod_delayed_work", ThreadAPIUtil::TYPE::FORK},
    {"flush_work", ThreadAPIUtil::TYPE::JOIN},  // waits for work to complete
    {"flush_delayed_work", ThreadAPIUtil::TYPE::JOIN},
    {"cancel_work_sync", ThreadAPIUtil::TYPE::JOIN},
    {"cancel_delayed_work_sync", ThreadAPIUtil::TYPE::JOIN},
    
    // Tasklets (soft IRQ context)
    {"tasklet_schedule", ThreadAPIUtil::TYPE::FORK},
    {"tasklet_hi_schedule", ThreadAPIUtil::TYPE::FORK},
    {"tasklet_kill", ThreadAPIUtil::TYPE::JOIN},
    {"tasklet_disable", ThreadAPIUtil::TYPE::JOIN},
    
    // Timers (async callback)
    {"mod_timer", ThreadAPIUtil::TYPE::FORK},
    {"add_timer", ThreadAPIUtil::TYPE::FORK},
    {"del_timer_sync", ThreadAPIUtil::TYPE::JOIN},
    {"timer_delete_sync", ThreadAPIUtil::TYPE::JOIN},
    
    // RCU (Read-Copy-Update)
    {"call_rcu", ThreadAPIUtil::TYPE::FORK},
    {"call_srcu", ThreadAPIUtil::TYPE::FORK},
    {"synchronize_rcu", ThreadAPIUtil::TYPE::JOIN},
    {"synchronize_srcu", ThreadAPIUtil::TYPE::JOIN},
    {"rcu_barrier", ThreadAPIUtil::TYPE::JOIN},
    
    // Interrupt handlers (top-half/bottom-half)
    {"request_irq", ThreadAPIUtil::TYPE::FORK},
    {"request_threaded_irq", ThreadAPIUtil::TYPE::FORK},
    {"free_irq", ThreadAPIUtil::TYPE::JOIN},
    
    // Completions (kernel wait/signal)
    {"complete", ThreadAPIUtil::TYPE::COND_SIGNAL},
    {"complete_all", ThreadAPIUtil::TYPE::COND_BROADCAST},
    {"wait_for_completion", ThreadAPIUtil::TYPE::COND_WAIT},
    {"wait_for_completion_timeout", ThreadAPIUtil::TYPE::COND_WAIT},
    {"wait_for_completion_interruptible", ThreadAPIUtil::TYPE::COND_WAIT},
    
    //==========================================================================
    // POSIX Thread Join/Exit/Detach
    //==========================================================================
    {"pthread_join", ThreadAPIUtil::TYPE::JOIN},
    {"pthread_cancel", ThreadAPIUtil::TYPE::JOIN},
    {"vdbeSorterJoinThread", ThreadAPIUtil::TYPE::JOIN},
    {"MtSync_Destruct", ThreadAPIUtil::TYPE::JOIN},
    {"kthread_stop", ThreadAPIUtil::TYPE::JOIN},
    
    //==========================================================================
    // Mutex/Lock Acquire
    //==========================================================================
    {"pthread_mutex_lock", ThreadAPIUtil::TYPE::ACQUIRE},
    {"pthread_rwlock_rdlock", ThreadAPIUtil::TYPE::ACQUIRE},
    {"pthread_rwlock_wrlock", ThreadAPIUtil::TYPE::ACQUIRE},
    {"sem_wait", ThreadAPIUtil::TYPE::ACQUIRE},
    {"_spin_lock", ThreadAPIUtil::TYPE::ACQUIRE},
    {"release_", ThreadAPIUtil::TYPE::ACQUIRE},
    {"SRE_SplSpecLockEx", ThreadAPIUtil::TYPE::ACQUIRE},
    // Linux kernel locks
    {"mutex_lock", ThreadAPIUtil::TYPE::ACQUIRE},
    {"mutex_lock_nested", ThreadAPIUtil::TYPE::ACQUIRE},
    {"mutex_lock_interruptible", ThreadAPIUtil::TYPE::ACQUIRE},
    {"mutex_lock_interruptible_nested", ThreadAPIUtil::TYPE::ACQUIRE},
    {"mutex_lock_killable", ThreadAPIUtil::TYPE::ACQUIRE},
    {"mutex_lock_killable_nested", ThreadAPIUtil::TYPE::ACQUIRE},
    {"mutex_lock_io", ThreadAPIUtil::TYPE::ACQUIRE},
    {"ww_mutex_lock", ThreadAPIUtil::TYPE::ACQUIRE},
    {"ww_mutex_lock_interruptible", ThreadAPIUtil::TYPE::ACQUIRE},
    {"spin_lock", ThreadAPIUtil::TYPE::ACQUIRE},
    {"spin_lock_nested", ThreadAPIUtil::TYPE::ACQUIRE},
    {"spin_lock_irq", ThreadAPIUtil::TYPE::ACQUIRE},
    {"spin_lock_irqsave", ThreadAPIUtil::TYPE::ACQUIRE},
    {"spin_lock_irqsave_nested", ThreadAPIUtil::TYPE::ACQUIRE},
    {"spin_lock_bh", ThreadAPIUtil::TYPE::ACQUIRE},
    {"_raw_spin_lock", ThreadAPIUtil::TYPE::ACQUIRE},
    {"_raw_spin_lock_irq", ThreadAPIUtil::TYPE::ACQUIRE},
    {"_raw_spin_lock_irqsave", ThreadAPIUtil::TYPE::ACQUIRE},
    {"_raw_spin_lock_bh", ThreadAPIUtil::TYPE::ACQUIRE},
    {"raw_spin_lock", ThreadAPIUtil::TYPE::ACQUIRE},
    {"raw_spin_lock_irq", ThreadAPIUtil::TYPE::ACQUIRE},
    {"raw_spin_lock_irqsave", ThreadAPIUtil::TYPE::ACQUIRE},
    {"raw_spin_lock_bh", ThreadAPIUtil::TYPE::ACQUIRE},
    {"read_lock", ThreadAPIUtil::TYPE::ACQUIRE},
    {"read_lock_irq", ThreadAPIUtil::TYPE::ACQUIRE},
    {"read_lock_irqsave", ThreadAPIUtil::TYPE::ACQUIRE},
    {"read_lock_bh", ThreadAPIUtil::TYPE::ACQUIRE},
    {"write_lock", ThreadAPIUtil::TYPE::ACQUIRE},
    {"write_lock_irq", ThreadAPIUtil::TYPE::ACQUIRE},
    {"write_lock_irqsave", ThreadAPIUtil::TYPE::ACQUIRE},
    {"write_lock_bh", ThreadAPIUtil::TYPE::ACQUIRE},
    {"down", ThreadAPIUtil::TYPE::ACQUIRE},
    {"down_interruptible", ThreadAPIUtil::TYPE::ACQUIRE},
    {"down_killable", ThreadAPIUtil::TYPE::ACQUIRE},
    {"down_read", ThreadAPIUtil::TYPE::ACQUIRE},
    {"down_read_interruptible", ThreadAPIUtil::TYPE::ACQUIRE},
    {"down_read_killable", ThreadAPIUtil::TYPE::ACQUIRE},
    {"down_read_nested", ThreadAPIUtil::TYPE::ACQUIRE},
    {"down_write", ThreadAPIUtil::TYPE::ACQUIRE},
    {"down_write_killable", ThreadAPIUtil::TYPE::ACQUIRE},
    {"down_write_nested", ThreadAPIUtil::TYPE::ACQUIRE},
    {"rcu_read_lock", ThreadAPIUtil::TYPE::ACQUIRE},
    {"rcu_read_lock_bh", ThreadAPIUtil::TYPE::ACQUIRE},
    {"rcu_read_lock_sched", ThreadAPIUtil::TYPE::ACQUIRE},
    {"rcu_read_lock_sched_notrace", ThreadAPIUtil::TYPE::ACQUIRE},
    {"srcu_read_lock", ThreadAPIUtil::TYPE::ACQUIRE},
    {"srcu_read_lock_nmisafe", ThreadAPIUtil::TYPE::ACQUIRE},
    {"local_bh_disable", ThreadAPIUtil::TYPE::ACQUIRE},
    {"local_irq_disable", ThreadAPIUtil::TYPE::ACQUIRE},
    {"local_irq_save", ThreadAPIUtil::TYPE::ACQUIRE},
    {"preempt_disable", ThreadAPIUtil::TYPE::ACQUIRE},
    {"preempt_disable_notrace", ThreadAPIUtil::TYPE::ACQUIRE},
    {"get_online_cpus", ThreadAPIUtil::TYPE::ACQUIRE},
    {"cpus_read_lock", ThreadAPIUtil::TYPE::ACQUIRE},
    {"rtnl_lock", ThreadAPIUtil::TYPE::ACQUIRE},
    {"netlink_lock_table", ThreadAPIUtil::TYPE::ACQUIRE},
    {"seq_spin_lock", ThreadAPIUtil::TYPE::ACQUIRE},
    {"write_seqlock", ThreadAPIUtil::TYPE::ACQUIRE},
    {"write_seqlock_irq", ThreadAPIUtil::TYPE::ACQUIRE},
    {"write_seqlock_irqsave", ThreadAPIUtil::TYPE::ACQUIRE},
    {"write_seqlock_bh", ThreadAPIUtil::TYPE::ACQUIRE},
    {"read_seqlock_excl", ThreadAPIUtil::TYPE::ACQUIRE},
    {"raw_read_seqcount_begin", ThreadAPIUtil::TYPE::ACQUIRE},
    {"bit_spin_lock", ThreadAPIUtil::TYPE::ACQUIRE},
    
    //==========================================================================
    // Mutex/Lock Try-Acquire
    //==========================================================================
    {"pthread_mutex_trylock", ThreadAPIUtil::TYPE::TRY_ACQUIRE},
    {"mutex_trylock", ThreadAPIUtil::TYPE::TRY_ACQUIRE},
    {"spin_trylock", ThreadAPIUtil::TYPE::TRY_ACQUIRE},
    {"spin_trylock_irq", ThreadAPIUtil::TYPE::TRY_ACQUIRE},
    {"spin_trylock_irqsave", ThreadAPIUtil::TYPE::TRY_ACQUIRE},
    {"spin_trylock_bh", ThreadAPIUtil::TYPE::TRY_ACQUIRE},
    {"raw_spin_trylock", ThreadAPIUtil::TYPE::TRY_ACQUIRE},
    {"_raw_spin_trylock", ThreadAPIUtil::TYPE::TRY_ACQUIRE},
    {"read_trylock", ThreadAPIUtil::TYPE::TRY_ACQUIRE},
    {"write_trylock", ThreadAPIUtil::TYPE::TRY_ACQUIRE},
    {"down_trylock", ThreadAPIUtil::TYPE::TRY_ACQUIRE},
    {"down_read_trylock", ThreadAPIUtil::TYPE::TRY_ACQUIRE},
    {"down_write_trylock", ThreadAPIUtil::TYPE::TRY_ACQUIRE},
    {"rtnl_trylock", ThreadAPIUtil::TYPE::TRY_ACQUIRE},
    {"ww_mutex_trylock", ThreadAPIUtil::TYPE::TRY_ACQUIRE},
    
    //==========================================================================
    // Mutex/Lock Release
    //==========================================================================
    {"pthread_mutex_unlock", ThreadAPIUtil::TYPE::RELEASE},
    {"possess_", ThreadAPIUtil::TYPE::RELEASE},
    {"pthread_rwlock_unlock", ThreadAPIUtil::TYPE::RELEASE},
    {"sem_post", ThreadAPIUtil::TYPE::RELEASE},
    {"_spin_unlock", ThreadAPIUtil::TYPE::RELEASE},
    {"SRE_SplSpecUnlockEx", ThreadAPIUtil::TYPE::RELEASE},
    // Linux kernel unlocks
    {"mutex_unlock", ThreadAPIUtil::TYPE::RELEASE},
    {"ww_mutex_unlock", ThreadAPIUtil::TYPE::RELEASE},
    {"spin_unlock", ThreadAPIUtil::TYPE::RELEASE},
    {"spin_unlock_irq", ThreadAPIUtil::TYPE::RELEASE},
    {"spin_unlock_irqrestore", ThreadAPIUtil::TYPE::RELEASE},
    {"spin_unlock_bh", ThreadAPIUtil::TYPE::RELEASE},
    {"_raw_spin_unlock", ThreadAPIUtil::TYPE::RELEASE},
    {"_raw_spin_unlock_irq", ThreadAPIUtil::TYPE::RELEASE},
    {"_raw_spin_unlock_irqrestore", ThreadAPIUtil::TYPE::RELEASE},
    {"_raw_spin_unlock_bh", ThreadAPIUtil::TYPE::RELEASE},
    {"raw_spin_unlock", ThreadAPIUtil::TYPE::RELEASE},
    {"raw_spin_unlock_irq", ThreadAPIUtil::TYPE::RELEASE},
    {"raw_spin_unlock_irqrestore", ThreadAPIUtil::TYPE::RELEASE},
    {"raw_spin_unlock_bh", ThreadAPIUtil::TYPE::RELEASE},
    {"read_unlock", ThreadAPIUtil::TYPE::RELEASE},
    {"read_unlock_irq", ThreadAPIUtil::TYPE::RELEASE},
    {"read_unlock_irqrestore", ThreadAPIUtil::TYPE::RELEASE},
    {"read_unlock_bh", ThreadAPIUtil::TYPE::RELEASE},
    {"write_unlock", ThreadAPIUtil::TYPE::RELEASE},
    {"write_unlock_irq", ThreadAPIUtil::TYPE::RELEASE},
    {"write_unlock_irqrestore", ThreadAPIUtil::TYPE::RELEASE},
    {"write_unlock_bh", ThreadAPIUtil::TYPE::RELEASE},
    {"up", ThreadAPIUtil::TYPE::RELEASE},
    {"up_read", ThreadAPIUtil::TYPE::RELEASE},
    {"up_write", ThreadAPIUtil::TYPE::RELEASE},
    {"rcu_read_unlock", ThreadAPIUtil::TYPE::RELEASE},
    {"rcu_read_unlock_bh", ThreadAPIUtil::TYPE::RELEASE},
    {"rcu_read_unlock_sched", ThreadAPIUtil::TYPE::RELEASE},
    {"rcu_read_unlock_sched_notrace", ThreadAPIUtil::TYPE::RELEASE},
    {"srcu_read_unlock", ThreadAPIUtil::TYPE::RELEASE},
    {"srcu_read_unlock_nmisafe", ThreadAPIUtil::TYPE::RELEASE},
    {"local_bh_enable", ThreadAPIUtil::TYPE::RELEASE},
    {"local_irq_enable", ThreadAPIUtil::TYPE::RELEASE},
    {"local_irq_restore", ThreadAPIUtil::TYPE::RELEASE},
    {"preempt_enable", ThreadAPIUtil::TYPE::RELEASE},
    {"preempt_enable_notrace", ThreadAPIUtil::TYPE::RELEASE},
    {"put_online_cpus", ThreadAPIUtil::TYPE::RELEASE},
    {"cpus_read_unlock", ThreadAPIUtil::TYPE::RELEASE},
    {"rtnl_unlock", ThreadAPIUtil::TYPE::RELEASE},
    {"netlink_unlock_table", ThreadAPIUtil::TYPE::RELEASE},
    {"seq_spin_unlock", ThreadAPIUtil::TYPE::RELEASE},
    {"write_sequnlock", ThreadAPIUtil::TYPE::RELEASE},
    {"write_sequnlock_irq", ThreadAPIUtil::TYPE::RELEASE},
    {"write_sequnlock_irqrestore", ThreadAPIUtil::TYPE::RELEASE},
    {"write_sequnlock_bh", ThreadAPIUtil::TYPE::RELEASE},
    {"read_sequnlock_excl", ThreadAPIUtil::TYPE::RELEASE},
    {"bit_spin_unlock", ThreadAPIUtil::TYPE::RELEASE},
    
    //==========================================================================
    // Thread Exit/Detach
    //==========================================================================
    {"pthread_exit", ThreadAPIUtil::TYPE::EXIT},
    {"pthread_detach", ThreadAPIUtil::TYPE::DETACH},
    {"kthread_exit", ThreadAPIUtil::TYPE::EXIT},
    
    //==========================================================================
    // Condition Variables
    //==========================================================================
    {"pthread_cond_wait", ThreadAPIUtil::TYPE::COND_WAIT},
    {"pthread_cond_signal", ThreadAPIUtil::TYPE::COND_SIGNAL},
    {"pthread_cond_broadcast", ThreadAPIUtil::TYPE::COND_BROADCAST},
    {"pthread_cond_init", ThreadAPIUtil::TYPE::CONDVAR_INI},
    {"pthread_cond_destroy", ThreadAPIUtil::TYPE::CONDVAR_DESTROY},
    // Linux kernel wait queues
    {"wait_event", ThreadAPIUtil::TYPE::COND_WAIT},
    {"wait_event_interruptible", ThreadAPIUtil::TYPE::COND_WAIT},
    {"wait_event_timeout", ThreadAPIUtil::TYPE::COND_WAIT},
    {"wake_up", ThreadAPIUtil::TYPE::COND_SIGNAL},
    {"wake_up_all", ThreadAPIUtil::TYPE::COND_BROADCAST},
    {"wake_up_interruptible", ThreadAPIUtil::TYPE::COND_SIGNAL},
    {"wake_up_interruptible_all", ThreadAPIUtil::TYPE::COND_BROADCAST},
    
    //==========================================================================
    // Mutex Init/Destroy
    //==========================================================================
    {"pthread_mutex_init", ThreadAPIUtil::TYPE::MUTEX_INI},
    {"pthread_mutex_destroy", ThreadAPIUtil::TYPE::MUTEX_DESTROY},
    {"mutex_init", ThreadAPIUtil::TYPE::MUTEX_INI},
    {"mutex_destroy", ThreadAPIUtil::TYPE::MUTEX_DESTROY},
    {"spin_lock_init", ThreadAPIUtil::TYPE::MUTEX_INI},
    
    //==========================================================================
    // Barrier
    //==========================================================================
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

void ThreadAPIUtil::initEntryArgIndices() {
    // Helper lambda to create single-element vector
    auto v1 = [](int i) { return std::vector<int>{i}; };
    auto v2 = [](int i, int j) { return std::vector<int>{i, j}; };
    
    // POSIX pthread_create: int pthread_create(pthread_t *thread, const pthread_attr_t *attr, 
    //                                          void *(*start_routine)(void *), void *arg)
    // Entry function is at argument index 3 (0-based: 2)
    entryArgIndexMap["pthread_create"] = v1(3);
    entryArgIndexMap["celixThread_create"] = v1(3);
    entryArgIndexMap["apr_thread_create"] = v1(3);
    
    // Linux kernel kthread: struct task_struct *kthread_create(int (*threadfn)(void *data), 
    //                                                          void *data, const char namefmt[], ...)
    // Entry function is at argument index 1 (0-based: 0)
    entryArgIndexMap["kthread_create"] = v1(1);
    entryArgIndexMap["kthread_create_on_node"] = v1(1);
    entryArgIndexMap["kthread_create_on_cpu"] = v1(1);
    entryArgIndexMap["kthread_run"] = v1(1);
    entryArgIndexMap["kthread_create_worker"] = v1(1);
    entryArgIndexMap["kthread_create_worker_on_cpu"] = v1(2);  // cpu, flags, func, data, namefmt
    
    // Linux kernel workqueue: INIT_WORK(work, func) / queue_work(wq, work)
    // Work function is typically at argument 2 for INIT_WORK
    entryArgIndexMap["INIT_WORK"] = v1(2);
    entryArgIndexMap["INIT_DELAYED_WORK"] = v1(2);
    // For queue_work, the function is embedded in the work_struct, need LLM analysis
    entryArgIndexMap["queue_work"] = v1(2);  // work_struct contains the function
    entryArgIndexMap["queue_delayed_work"] = v1(3);
    entryArgIndexMap["schedule_work"] = v1(1);
    entryArgIndexMap["schedule_delayed_work"] = v1(1);
    
    // Linux kernel tasklet: tasklet_init(tasklet, func, data)
    entryArgIndexMap["tasklet_init"] = v1(2);
    entryArgIndexMap["tasklet_setup"] = v1(2);
    
    // Linux kernel timer: timer_setup(timer, callback, flags)
    entryArgIndexMap["timer_setup"] = v1(2);
    entryArgIndexMap["setup_timer"] = v1(2);  // older API
    
    // Linux kernel RCU: call_rcu(head, func)
    entryArgIndexMap["call_rcu"] = v1(2);
    entryArgIndexMap["call_srcu"] = v1(3);  // srcu_struct, rcu_head, func
    
    // Linux kernel IRQ: request_irq(irq, handler, flags, name, dev)
    entryArgIndexMap["request_irq"] = v1(2);
    entryArgIndexMap["request_threaded_irq"] = v2(2, 3);  // handler at 2, thread_fn at 3
    
    // LevelDB/common patterns
    entryArgIndexMap["StartThread"] = v1(1);  // StartThread(func, arg)
    entryArgIndexMap["Schedule"] = v1(1);
    
    // SQLite
    entryArgIndexMap["vdbeSorterCreateThread"] = v1(2);
}

std::vector<int> ThreadAPIUtil::getEntryFunctionArgIndices(const std::string& apiName) const {
    auto it = entryArgIndexMap.find(apiName);
    if (it != entryArgIndexMap.end()) {
        return it->second;
    }
    // 默认尝试位置：3 (pthread_create), 1, 2
    std::vector<int> defaultIndices;
    defaultIndices.push_back(3);
    defaultIndices.push_back(1);
    defaultIndices.push_back(2);
    return defaultIndices;
}

void ThreadAPIUtil::addEntryArgIndex(const std::string& apiName, const std::vector<int>& indices) {
    entryArgIndexMap[apiName] = indices;
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
    return isLockWrapperByName(callName);
}

bool ThreadAPIUtil::isTryAcquire(Node* node)
{
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);
    std::string callName = callNode->getName();
    if(threadAPIMap.find(callName) != threadAPIMap.end()){
        return threadAPIMap[callName] == ThreadAPIUtil::TYPE::TRY_ACQUIRE;
    }
    return isTryLockWrapperByName(callName);
}

bool ThreadAPIUtil::isRelease(Node* node)
{
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);
    std::string callName = callNode->getName();
    if(threadAPIMap.find(callName) != threadAPIMap.end()){
        return threadAPIMap[callName] == ThreadAPIUtil::TYPE::RELEASE;
    }
    return isUnlockWrapperByName(callName);
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

bool ThreadAPIUtil::isDelete(Node* node)
{
    if(node->getType() != "Call") return false;
    CallNode* callNode = static_cast<CallNode*>(node);
    return callNode->getName() == "<operator>.delete";
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
           isReturn(node) ||
           isDelete(node);
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



