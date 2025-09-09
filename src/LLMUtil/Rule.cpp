// src/LLMUtil/Rule.cpp

#include "LLMUtil/Rule.h"
#include "Query/StatefulBugDetector.h"
#include "CCPG/ThreadCreationTree.h"
#include "CCPG/AliasChecker.h"
#include <queue>
#include <set>

namespace llm_client {

/**
 * @brief Checks if a control flow path exists from a start node to an end node within a single thread.
 * @return True if a path exists, false otherwise.
 */
bool is_reachable_intra_thread(CCPGNode* start_node, CCPGNode* end_node, Thread* thread) {
    if (!start_node || !end_node || !thread) return false;
    if (start_node == end_node) return true;

    if (thread->getNodes().find(start_node) == thread->getNodes().end() ||
        thread->getNodes().find(end_node) == thread->getNodes().end()) {
        return false;
    }

    std::queue<CCPGNode*> worklist;
    std::set<CCPGNode*> visited;

    worklist.push(start_node);
    visited.insert(start_node);

    while (!worklist.empty()) {
        CCPGNode* current = worklist.front();
        worklist.pop();

        for (CCPGEdge* edge : current->getOutEdges()) {
            if (edge->getType() == CCPGEdge::EdgeType::ORDER) {
                CCPGNode* next = edge->getDst();
                if (next == end_node) {
                    return true;
                }
                if (thread->getNodes().count(next) && visited.find(next) == visited.end()) {
                    visited.insert(next);
                    worklist.push(next);
                }
            }
        }
    }
    return false;
}

// --- Rule Verification Implementations ---

std::optional<query::StatefulBug> TOCTOURule::verify(const ThreadPair& pair, CCPG* ccpg) const {
    ThreadCreationTree* tct = ThreadCreationTree::getInstance();
    
    CCPGNode* check_node = ccpg->getNodeByID(get_node_for_role("state_check_operation"));
    CCPGNode* modify_node = ccpg->getNodeByID(get_node_for_role("state_modify_operation"));
    CCPGNode* use_node = ccpg->getNodeByID(get_node_for_role("resource_use_operation"));

    if (!check_node || !modify_node || !use_node) return std::nullopt;

    Thread *checker_thread = nullptr, *modifier_thread = nullptr;
    if (pair.thread1->getNodes().count(check_node) && pair.thread2->getNodes().count(modify_node)) {
        checker_thread = pair.thread1; modifier_thread = pair.thread2;
    } else if (pair.thread2->getNodes().count(check_node) && pair.thread1->getNodes().count(modify_node)) {
        checker_thread = pair.thread2; modifier_thread = pair.thread1;
    } else {
        return std::nullopt;
    }

    if (tct->mayThreadsRunConcurrently(checker_thread, modifier_thread) &&
        is_reachable_intra_thread(check_node, use_node, checker_thread)) {
        
        std::vector<std::pair<std::string, CCPGNode*>> path = {
            {"[CHECK] Operation", check_node},
            {"[MODIFY - Concurrent] Operation", modify_node},
            {"[USE] Operation", use_node}
        };
        return query::StatefulBug(this->to_json(), path, pair);
    }

    return std::nullopt;
}

std::optional<query::StatefulBug> DataRaceRule::verify(const ThreadPair& pair, CCPG* ccpg) const {
    ThreadCreationTree* tct = ThreadCreationTree::getInstance();

    CCPGNode* read_node = ccpg->getNodeByID(get_node_for_role("read_operation"));
    CCPGNode* write_node = ccpg->getNodeByID(get_node_for_role("write_operation"));

    if (!read_node || !write_node) return std::nullopt;
    
    if (tct->mayThreadsRunConcurrently(pair.thread1, pair.thread2)) {
		std::vector<std::pair<std::string, CCPGNode*>> path = {
			{"[READ] Operation", read_node},
			{"[WRITE - Concurrent] Operation", write_node}
		};
		return query::StatefulBug(this->to_json(), path, pair);
	}
    
    return std::nullopt;
}

std::optional<query::StatefulBug> UseAfterFreeRule::verify(const ThreadPair& pair, CCPG* ccpg) const {
    ThreadCreationTree* tct = ThreadCreationTree::getInstance();

    CCPGNode* free_node = ccpg->getNodeByID(get_node_for_role("free_operation"));
    CCPGNode* use_node = ccpg->getNodeByID(get_node_for_role("use_operation"));

    if (!free_node || !use_node) return std::nullopt;
    
    Thread *freer_thread = pair.thread1->getNodes().count(free_node) ? pair.thread1 : (pair.thread2->getNodes().count(free_node) ? pair.thread2 : nullptr);
    Thread *user_thread = pair.thread1->getNodes().count(use_node) ? pair.thread1 : (pair.thread2->getNodes().count(use_node) ? pair.thread2 : nullptr);

    if (!freer_thread || !user_thread) return std::nullopt;

    bool vulnerability_exists = false;
    if (freer_thread == user_thread) { // Intra-thread
        vulnerability_exists = is_reachable_intra_thread(free_node, use_node, freer_thread);
    } else { // Inter-thread
        vulnerability_exists = tct->mayThreadsRunConcurrently(freer_thread, user_thread);
    }

    if (vulnerability_exists) {
        std::vector<std::pair<std::string, CCPGNode*>> path = {
            {"[FREE] Operation", free_node},
            {"[USE] Operation", use_node}
        };
        return query::StatefulBug(this->to_json(), path, pair);
    }
    
    return std::nullopt;
}

std::optional<query::StatefulBug> DoubleFreeRule::verify(const ThreadPair& pair, CCPG* ccpg) const {
    ThreadCreationTree* tct = ThreadCreationTree::getInstance();

    CCPGNode* first_free = ccpg->getNodeByID(get_node_for_role("first_free_operation"));
    CCPGNode* second_free = ccpg->getNodeByID(get_node_for_role("second_free_operation"));

    if (!first_free || !second_free) return std::nullopt;

    Thread *first_freer = pair.thread1->getNodes().count(first_free) ? pair.thread1 : (pair.thread2->getNodes().count(first_free) ? pair.thread2 : nullptr);
    Thread *second_freer = pair.thread1->getNodes().count(second_free) ? pair.thread1 : (pair.thread2->getNodes().count(second_free) ? pair.thread2 : nullptr);

    if (!first_freer || !second_freer) return std::nullopt;

    bool vulnerability_exists = false;
    if (first_freer == second_freer) { // Intra-thread
        vulnerability_exists = is_reachable_intra_thread(first_free, second_free, first_freer);
    } else { // Inter-thread
        vulnerability_exists = tct->mayThreadsRunConcurrently(first_freer, second_freer);
    }
    
    if(vulnerability_exists){
        std::vector<std::pair<std::string, CCPGNode*>> path = {
            {"[FIRST FREE] Operation", first_free},
            {"[SECOND FREE] Operation", second_free}
        };
        return query::StatefulBug(this->to_json(), path, pair);
    }

    return std::nullopt;
}

std::optional<query::StatefulBug> NullPointerDereferenceRule::verify(const ThreadPair& pair, CCPG* ccpg) const {
    ThreadCreationTree* tct = ThreadCreationTree::getInstance();
    AliasChecker* aliasChecker = AliasChecker::getInstance();

    CCPGNode* null_assignment = ccpg->getNodeByID(get_node_for_role("null_assignment_operation"));
    CCPGNode* dereference = ccpg->getNodeByID(get_node_for_role("dereference_operation"));

    if (!null_assignment || !dereference) return std::nullopt;

    Thread *assigner_thread = pair.thread1->getNodes().count(null_assignment) ? pair.thread1 : (pair.thread2->getNodes().count(null_assignment) ? pair.thread2 : nullptr);
    Thread *dereferencer_thread = pair.thread1->getNodes().count(dereference) ? pair.thread1 : (pair.thread2->getNodes().count(dereference) ? pair.thread2 : nullptr);

    if (!assigner_thread || !dereferencer_thread) return std::nullopt;

    bool vulnerability_exists = false;
    if (assigner_thread == dereferencer_thread) { // Intra-thread
        vulnerability_exists = is_reachable_intra_thread(null_assignment, dereference, assigner_thread);
    } else { // Inter-thread
        vulnerability_exists = tct->mayThreadsRunConcurrently(assigner_thread, dereferencer_thread);
    }

    if (vulnerability_exists) {
        auto assign_accesses = aliasChecker->getMemoryAccessesFromLocation(null_assignment->getNodeLoc(), null_assignment->getFunction()->getContextSet().empty() ? Context() : **null_assignment->getFunction()->getContextSet().begin());
        auto deref_accesses = aliasChecker->getMemoryAccessesFromLocation(dereference->getNodeLoc(), dereference->getFunction()->getContextSet().empty() ? Context() : **dereference->getFunction()->getContextSet().begin());
        
        for (const auto& assign_acc : assign_accesses) {
            for (const auto& deref_acc : deref_accesses) {
                if (aliasChecker->isAlias(assign_acc.pointerOperand, deref_acc.pointerOperand)) {
                    std::vector<std::pair<std::string, CCPGNode*>> path = {
                        {"[NULL ASSIGNMENT] Operation", null_assignment},
                        {"[DEREFERENCE] Operation", dereference}
                    };
                    return query::StatefulBug(this->to_json(), path, pair);
                }
            }
        }
    }
    
    return std::nullopt;
}

std::optional<query::StatefulBug> DeadlockRule::verify(const ThreadPair& pair, CCPG* ccpg) const {
    ThreadCreationTree* tct = ThreadCreationTree::getInstance();
    AliasChecker* aliasChecker = AliasChecker::getInstance();

    CCPGNode* t1_lock1 = ccpg->getNodeByID(get_node_for_role("thread1_first_lock"));
    CCPGNode* t1_lock2 = ccpg->getNodeByID(get_node_for_role("thread1_second_lock"));
    CCPGNode* t2_lock1 = ccpg->getNodeByID(get_node_for_role("thread2_first_lock"));
    CCPGNode* t2_lock2 = ccpg->getNodeByID(get_node_for_role("thread2_second_lock"));

    if (!t1_lock1 || !t1_lock2 || !t2_lock1 || !t2_lock2) return std::nullopt;
    
    if (tct->mayThreadsRunConcurrently(pair.thread1, pair.thread2) &&
        is_reachable_intra_thread(t1_lock1, t1_lock2, pair.thread1) &&
        is_reachable_intra_thread(t2_lock1, t2_lock2, pair.thread2) &&
        aliasChecker->isLockAlias(t1_lock1, t2_lock2) &&
        aliasChecker->isLockAlias(t1_lock2, t2_lock1) &&
        !aliasChecker->isLockAlias(t1_lock1, t1_lock2)) {

        std::vector<std::pair<std::string, CCPGNode*>> path = {
            {"[Thread 1] Acquires Lock A", t1_lock1},
            {"[Thread 2] Acquires Lock B", t2_lock1},
            {"[Thread 1] Waits for Lock B", t1_lock2},
            {"[Thread 2] Waits for Lock A", t2_lock2}
        };
        return query::StatefulBug(this->to_json(), path, pair);
    }

    return std::nullopt;
}
} // namespace llm_client