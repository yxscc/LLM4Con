#include "CCPG/HBGraph.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "CCPG/AliasChecker.h"
#include "CCPG/CCPG.h"
#include "CCPG/LSAnalysis.h"
#include "CCPG/ThreadCreationTree.h"
#include "CCPG/ThreadAPIUtil.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"

using ccpg::HBEdge;
using ccpg::HBEdgeKind;

namespace {

int envInt(const char* key, int def) {
  const char* s = std::getenv(key);
  if (!s || !*s) return def;
  return std::atoi(s);
}

const char* kindLabel(HBEdgeKind k) {
  switch (k) {
    case HBEdgeKind::PROGRAM_ORDER: return "PO";
    case HBEdgeKind::CALL_RETURN: return "CALL";
    case HBEdgeKind::LOCK_RELEASE_ACQUIRE: return "LOCK";
    case HBEdgeKind::FORK_TO_ENTRY: return "FORK";
    case HBEdgeKind::JOIN_FROM_EXIT: return "JOIN";
    case HBEdgeKind::RCU_SYNC: return "RCU";
    case HBEdgeKind::REFCOUNT_LAST_PUT: return "REFCNT";
    case HBEdgeKind::REL_ACQ_PAIR: return "REL_ACQ";
    case HBEdgeKind::BH_IRQ_INTERVAL: return "BH";
    case HBEdgeKind::COMPLETION: return "COMP";
    case HBEdgeKind::LIFECYCLE_FLAG: return "LIFE";
  }
  return "?";
}

const llvm::Value* getCallArg0(CCPGNode* n) {
  if (!n || !n->isCallSite()) return nullptr;
  const llvm::CallInst* c = n->getLLVMCallInst();
  if (!c || c->arg_size() == 0) return nullptr;
  return c->getArgOperand(0);
}

bool completionAlias(CCPGNode* a, CCPGNode* b) {
  if (!a || !b) return false;
  const llvm::Value* va = getCallArg0(a);
  const llvm::Value* vb = getCallArg0(b);
  if (!va || !vb) return false;
  return AliasChecker::getInstance()->isAlias(va, vb);
}

} // namespace

HBGraph* HBGraph::instance = nullptr;

HBGraph* HBGraph::getInstance() {
  if (instance == nullptr) {
    instance = new HBGraph();
  }
  return instance;
}

void HBGraph::clear() {
  outEdges_.clear();
  inEdges_.clear();
}

void HBGraph::addEdge(CCPGNode* src, CCPGNode* dst, HBEdgeKind kind,
                      const std::string& evidence) {
  if (!src || !dst) return;
  HBEdge e{src, dst, kind, evidence};
  outEdges_[src].push_back(e);
  inEdges_[dst].push_back(e);
}

void HBGraph::build(CCPG* ccpg) {
  if (!ccpg) return;
  clear();
  buildProgramOrderEdges(ccpg);
  buildLockEdges(ccpg);
  buildForkJoinEdges(ccpg);
  buildCompletionEdges(ccpg);
  buildLifecycleFlagEdges(ccpg);
  buildRCUEdges(ccpg);
}

void HBGraph::buildProgramOrderEdges(CCPG* ccpg) {
  for (CCPGEdge* e : ccpg->getEdges()) {
    CCPGNode* s = e->getSrc();
    CCPGNode* d = e->getDst();
    if (!s || !d) continue;
    switch (e->getType()) {
      case CCPGEdge::EdgeType::ORDER:
        addEdge(s, d, HBEdgeKind::PROGRAM_ORDER, "cfg");
        break;
      case CCPGEdge::EdgeType::CALL:
        addEdge(s, d, HBEdgeKind::CALL_RETURN, "call");
        break;
      case CCPGEdge::EdgeType::HB:
        addEdge(s, d, HBEdgeKind::FORK_TO_ENTRY, "fork");
        break;
    }
  }
}

void HBGraph::buildLockEdges(CCPG* ccpg) {
  (void)ccpg;
  auto* ls = LSAnalysis::getInstance();
  auto* ac = AliasChecker::getInstance();
  if (!ls || !ac) return;

  std::vector<std::vector<Lock*>> classes;
  for (Lock* l : ls->getLocks()) {
    if (!l->getAcquire()) continue;
    bool placed = false;
    for (auto& cls : classes) {
      if (cls.empty()) continue;
      if (ac->isLockAlias(cls.front()->getAcquire(), l->getAcquire())) {
        cls.push_back(l);
        placed = true;
        break;
      }
    }
    if (!placed) {
      classes.push_back({l});
    }
  }

  auto* tct = ThreadCreationTree::getInstance();
  if (!tct) return;

  auto findThread = [&](CCPGNode* n) -> Thread* {
    for (Thread* t : tct->getThreads()) {
      if (t->getNodes().count(n)) return t;
    }
    return nullptr;
  };

  for (auto& cls : classes) {
    for (Lock* lr : cls) {
      CCPGNode* relN = lr->getRelease();
      if (!relN) continue;
      Thread* tr = findThread(relN);
      for (Lock* la : cls) {
        if (la == lr) continue;
        CCPGNode* acqN = la->getAcquire();
        if (!acqN) continue;
        Thread* ta = findThread(acqN);
        if (!tr || !ta || tr == ta) continue;
        if (!tct->mayThreadsRunConcurrently(tr, ta)) continue;
        addEdge(relN, acqN, HBEdgeKind::LOCK_RELEASE_ACQUIRE,
                "lock" + std::to_string(lr->getId()));
      }
    }
  }
}

void HBGraph::buildForkJoinEdges(CCPG* ccpg) {
  (void)ccpg;
  auto* tct = ThreadCreationTree::getInstance();
  if (!tct) return;

  for (Thread* child : tct->getThreads()) {
    CCPGNode* join = child->getJoinNode();
    if (!join) continue;

    std::vector<CCPGNode*> exits;
    for (CCPGNode* n : child->getNodes()) {
      if (n->getType() == ThreadAPIUtil::TYPE::RETURN) {
        exits.push_back(n);
      }
    }
    if (exits.empty()) continue;

    for (CCPGEdge* oe : join->getOutEdges()) {
      if (oe->getType() != CCPGEdge::EdgeType::ORDER) continue;
      CCPGNode* next = oe->getDst();
      if (!next) continue;
      for (CCPGNode* ex : exits) {
        addEdge(ex, next, HBEdgeKind::JOIN_FROM_EXIT, "join");
      }
    }
  }
}

void HBGraph::buildCompletionEdges(CCPG* ccpg) {
  (void)ccpg;
  CCPG* g = LSAnalysis::getInstance()->getCCPG();
  if (!g) return;

  CCPGNodeSet signals = g->getNodesByType(ThreadAPIUtil::TYPE::COND_SIGNAL);
  CCPGNodeSet waits = g->getNodesByType(ThreadAPIUtil::TYPE::COND_WAIT);

  for (CCPGNode* c : signals) {
    for (CCPGNode* w : waits) {
      if (!completionAlias(c, w)) continue;
      for (CCPGEdge* oe : w->getOutEdges()) {
        if (oe->getType() != CCPGEdge::EdgeType::ORDER) continue;
        CCPGNode* nxt = oe->getDst();
        if (nxt) {
          addEdge(c, nxt, HBEdgeKind::COMPLETION, "complete->after_wait");
        }
      }
    }
  }
}

// P8c: RCU happens-before edges.
//
// Models the kernel guarantee that synchronize_rcu() (or its srcu / bh /
// sched / expedited variants and rcu_barrier) blocks until every RCU
// read-side critical section that started before the call completes.
// Concretely, after synchronize_rcu() returns the writer can safely free
// the old version because no concurrent reader is still observing it.
//
// Static modeling:
//
//   For every synchronize_rcu*-style call site `S` in writer thread `Tw`,
//   and for every concurrent reader thread `Tr` (Tr != Tw, may run in
//   parallel with Tw), and for every rcu_read_unlock*-style call site `U`
//   in `Tr`, we add:
//
//       U  --RCU_SYNC-->  S
//
//   This says "U happens-before the return of S". Combined with the
//   intra-thread program-order edges, every read inside the corresponding
//   read-side critical section is HB-before any code that program-order
//   follows S in Tw — exactly the writer's "free old version after sync"
//   guarantee.
//
// (v20 P8c-fix) The original v20 P8c attached every concurrent
// `rcu_read_unlock*` site to every `synchronize_rcu*` site irrespective
// of RCU variant or SRCU instance. This was advertised as "strictly
// correct: spurious HB only removes RCU-reader-vs-RCU-reader races". In
// practice the spurious edges also block writer-side races whose only
// shared point with a reader is membership in some unrelated rcu_read_*
// region, contributing to the v20 batch's +128 P3_constraint_failed FPs
// noted in the post-batch summary. We now require the unlock and the
// synchronize to belong to the SAME RCU family. SRCU additionally
// requires the same `srcu_struct*` operand (compared value-identically;
// a deeper alias-checker pass would tighten this further but isn't
// needed to fix the regressions we observed).
void HBGraph::buildRCUEdges(CCPG* ccpg) {
  if (!ccpg) return;
  auto* tct = ThreadCreationTree::getInstance();
  if (!tct) return;

  // RCU variant families. `synchronize_X` only orders against
  // `rcu_read_unlock_X` of the same family. Tasks-RCU has no read-side
  // primitive; we keep its synchronize sites bucketed but they never
  // pair with an unlock so they won't create edges.
  enum class Family { Vanilla, BH, Sched, SRCU, Tasks, Unknown };
  auto syncFamily = [](const std::string& n) {
    if (n == "synchronize_rcu" || n == "synchronize_rcu_expedited" ||
        n == "rcu_barrier") return Family::Vanilla;
    if (n == "synchronize_rcu_bh") return Family::BH;
    if (n == "synchronize_sched" || n == "synchronize_sched_expedited")
      return Family::Sched;
    if (n == "synchronize_srcu" || n == "synchronize_srcu_expedited")
      return Family::SRCU;
    if (n == "synchronize_rcu_tasks" || n == "synchronize_rcu_tasks_rude" ||
        n == "rcu_barrier_tasks" || n == "rcu_barrier_tasks_rude")
      return Family::Tasks;
    return Family::Unknown;
  };
  auto unlockFamily = [](const std::string& n) {
    if (n == "rcu_read_unlock") return Family::Vanilla;
    if (n == "rcu_read_unlock_bh") return Family::BH;
    if (n == "rcu_read_unlock_sched" ||
        n == "rcu_read_unlock_sched_notrace") return Family::Sched;
    if (n == "srcu_read_unlock" || n == "srcu_read_unlock_nmisafe")
      return Family::SRCU;
    return Family::Unknown;
  };

  auto nodeNameOf = [](CCPGNode* n) -> std::string {
    if (!n) return "";
    Node* cn = n->getCPGNode();
    if (!cn) return "";
    return cn->getName();
  };

  // For SRCU we additionally compare the first call argument
  // (`struct srcu_struct *ssp`). Two SRCU sites pair only if they
  // reference the same LLVM Value. This is conservative — equivalent
  // pointers reached through different operands won't match without a
  // proper alias query — but eliminates the obvious unrelated-instance
  // case and keeps build time fast.
  auto srcuOperand = [](CCPGNode* n) -> const llvm::Value* {
    if (!n) return nullptr;
    const llvm::CallInst* ci = n->getLLVMCallInst();
    if (!ci || ci->arg_size() == 0) return nullptr;
    return ci->getArgOperand(0)->stripPointerCasts();
  };

  // Bucket sync sites by family.
  std::unordered_map<int, std::vector<CCPGNode*>> syncByFamily;  // Family -> sites
  for (CCPGNode* n : ccpg->getNodesByType(ThreadAPIUtil::TYPE::JOIN)) {
    if (!n) continue;
    Family f = syncFamily(nodeNameOf(n));
    if (f == Family::Unknown) continue;
    syncByFamily[static_cast<int>(f)].push_back(n);
  }
  if (syncByFamily.empty()) return;

  // Bucket unlock sites by family AND by Thread.
  std::unordered_map<int,
      std::unordered_map<Thread*, std::vector<CCPGNode*>>> unlocksByFamilyThread;
  for (Thread* t : tct->getThreads()) {
    if (!t) continue;
    for (CCPGNode* n : t->getNodesByType(ThreadAPIUtil::TYPE::RELEASE)) {
      if (!n) continue;
      Family f = unlockFamily(nodeNameOf(n));
      if (f == Family::Unknown) continue;
      unlocksByFamilyThread[static_cast<int>(f)][t].push_back(n);
    }
  }
  if (unlocksByFamilyThread.empty()) return;

  auto findThread = [&](CCPGNode* n) -> Thread* {
    for (Thread* t : tct->getThreads()) {
      if (t && t->getNodes().count(n)) return t;
    }
    return nullptr;
  };

  const int cap = envInt("HB_RCU_MAX_EDGES", 4000);
  std::size_t edges_added = 0;
  std::size_t edges_skipped_srcu_instance = 0;

  for (auto& fkv : syncByFamily) {
    int fam = fkv.first;
    auto urangeIt = unlocksByFamilyThread.find(fam);
    if (urangeIt == unlocksByFamilyThread.end()) continue;  // no readers in this family
    auto& unlocksByThread = urangeIt->second;

    for (CCPGNode* s : fkv.second) {
      Thread* tw = findThread(s);
      const llvm::Value* sOperand = (fam == static_cast<int>(Family::SRCU))
                                        ? srcuOperand(s) : nullptr;
      for (auto& kv : unlocksByThread) {
        Thread* tr = kv.first;
        if (!tr || tr == tw) continue;
        if (tw && !tct->mayThreadsRunConcurrently(tw, tr)) continue;
        for (CCPGNode* u : kv.second) {
          if (static_cast<int>(edges_added) >= cap) break;
          // SRCU instance check.
          if (fam == static_cast<int>(Family::SRCU)) {
            const llvm::Value* uOperand = srcuOperand(u);
            if (!sOperand || !uOperand || sOperand != uOperand) {
              ++edges_skipped_srcu_instance;
              continue;
            }
          }
          addEdge(u, s, HBEdgeKind::RCU_SYNC, "rcu_unlock->synchronize");
          ++edges_added;
        }
        if (static_cast<int>(edges_added) >= cap) break;
      }
      if (static_cast<int>(edges_added) >= cap) break;
    }
    if (static_cast<int>(edges_added) >= cap) break;
  }

  if (envInt("HB_RCU_DEBUG", 0) && (edges_added > 0 || edges_skipped_srcu_instance > 0)) {
    std::printf("[HBGraph] RCU_SYNC edges added: %zu (skipped_srcu_instance=%zu)\n",
                edges_added, edges_skipped_srcu_instance);
  }
}

void HBGraph::buildRefcountEdges(CCPG*) {}
void HBGraph::buildContextIntervalEdges(CCPG*) {}

// LIFECYCLE_FLAG: registration → callback edges.
//
// Models the kernel idiom "callback can only fire after registration".
// When the host thread calls a registration API (request_irq,
// serdev_device_set_client_ops, *_register, queue_work, wake_up_process,
// etc.) with a callback function — either as a direct fn-pointer arg or
// inside a struct-of-ops global — we add an HB edge:
//
//   <registration_call_site>  --LIFECYCLE_FLAG-->  <callback_thread_entry>
//
// Combined with intra-thread PROGRAM_ORDER edges, this makes any write that
// program-order-precedes the registration happens-before any read inside
// the callback thread, eliminating a large class of spurious data-race
// reports against probe/init code (CVE-2024-35977 etc).
void HBGraph::buildLifecycleFlagEdges(CCPG* ccpg) {
  if (!ccpg) return;
  auto* tct = ThreadCreationTree::getInstance();
  if (!tct) return;

  // Registration / publication APIs whose callback the kernel can invoke
  // only after the call returns. Direct fn-pointer args and ops-struct
  // args are both handled below.
  static const std::unordered_set<std::string> kRegistrationAPIs = {
      // Direct function-pointer argument
      "request_irq", "request_threaded_irq", "devm_request_irq",
      "devm_request_threaded_irq", "devm_request_any_context_irq",
      "queue_work", "queue_work_on", "queue_delayed_work",
      "queue_delayed_work_on", "schedule_work", "schedule_delayed_work",
      "wake_up_process", "kthread_run", "kthread_create",
      "kthread_create_on_node", "kthread_create_on_cpu",
      "tasklet_init", "tasklet_setup", "timer_setup", "init_timer",
      "INIT_WORK", "INIT_DELAYED_WORK", "INIT_DEFERRABLE_WORK",
      "hrtimer_init", "setup_timer",
      // Ops-struct argument (registers a vtable of callbacks)
      "serdev_device_set_client_ops", "register_netdev",
      "register_netdevice", "register_chrdev", "register_chrdev_region",
      "misc_register", "device_register", "device_add",
      "platform_device_register", "input_register_device",
      "i2c_add_driver", "spi_register_driver", "register_filesystem",
      "cdev_add", "cdev_init", "video_register_device", "snd_device_new",
      "register_pernet_subsys", "register_pernet_device",
      "register_netevent_notifier", "register_inetaddr_notifier",
      "register_netdevice_notifier", "rtnl_register_module",
      "rtnl_register", "nl_table_register", "genl_register_family",
      "register_sysctl", "register_sysctl_table", "proc_create",
      "debugfs_create_file", "sysfs_create_group",
      // P8b: sysfs / kobject attribute publication. After any of these
      // returns, the function pointers stored in the attached attribute
      // group / device_attribute can fire from user-space at any time.
      // Adding them ensures probe/init-phase writes that program-order
      // precede the publication call get an HB edge to the callback
      // entry node, removing a major class of "init vs sysfs callback"
      // false positives without losing real races inside the publish
      // window.
      "device_add_groups", "devm_device_add_groups", "device_add_group",
      "devm_device_add_group", "device_create_file",
      "sysfs_create_files", "sysfs_create_link", "sysfs_create_bin_file",
      "kobject_uevent",  // notifies userspace; can immediately race
      "kobject_add", "kobject_init_and_add",
      // Devm wrappers that publish + enable
      "devm_serdev_device_open", "devm_input_register_device",
      "devm_watchdog_register_device", "devm_register_netdev",
      "devm_misc_register", "devm_thermal_zone_of_sensor_register",
      "devm_iio_device_register", "devm_hwmon_device_register",
      "devm_led_classdev_register",
      // Generic publication helpers
      "rcu_assign_pointer", "smp_store_release",
      // ChromeOS / specific drivers seen in test corpus
      "cros_ec_register",
  };

  // Map: thread-entry function name → Thread object (so we can find the
  // CCPG entry node for any function we discover as a callback target).
  std::unordered_map<std::string, Thread*> entryFnToThread;
  for (Thread* t : tct->getThreads()) {
    if (!t) continue;
    ccpg::Function* mainFn = t->getThreadMainFunction();
    if (!mainFn || !mainFn->getFuncNode() ||
        !mainFn->getFuncNode()->getCPGNode()) {
      continue;
    }
    entryFnToThread[mainFn->getFuncNode()->getCPGNode()->getName()] = t;
  }
  if (entryFnToThread.empty()) return;

  // For a Thread, return the first operational CCPG node (the first
  // ORDER successor of the function-node). Falls back to the function
  // node itself.
  auto getThreadEntryNode = [](Thread* t) -> CCPGNode* {
    if (!t) return nullptr;
    ccpg::Function* fn = t->getThreadMainFunction();
    if (!fn || !fn->getFuncNode()) return nullptr;
    CCPGNode* funcNode = fn->getFuncNode();
    for (CCPGEdge* oe : funcNode->getOutEdges()) {
      if (oe->getType() == CCPGEdge::EdgeType::ORDER) return oe->getDst();
    }
    return funcNode;
  };

  // Recursively collect every llvm::Function constant reachable through
  // a constant initializer (handles both the simple fn-pointer arg case
  // and the struct-of-ops case where ops are nested inside ConstantStruct
  // / ConstantArray / ConstantAggregate).
  //
  // Net-filter (and similar) ops-struct globals can nest extremely deep
  // and reference each other through ConstantExpr; an unbounded recursion
  // can blow the stack or loop on shared-Constant cycles. Hard-cap depth
  // and dedupe on visited Constants.
  static constexpr int kMaxConstDepth = 8;
  static constexpr unsigned kMaxOperands = 256;
  std::unordered_set<const llvm::Constant*> visitedConsts;
  std::function<void(const llvm::Constant*,
                     std::vector<const llvm::Function*>&, int)>
      collectFns = [&](const llvm::Constant* c,
                       std::vector<const llvm::Function*>& out,
                       int depth) {
        if (!c || depth > kMaxConstDepth) return;
        if (!visitedConsts.insert(c).second) return;
        const llvm::Value* stripped = c->stripPointerCasts();
        if (!stripped) return;
        if (const auto* fn = llvm::dyn_cast<llvm::Function>(stripped)) {
          out.push_back(fn);
          return;
        }
        const auto* cAfter = llvm::dyn_cast<llvm::Constant>(stripped);
        if (!cAfter) return;
        unsigned n = cAfter->getNumOperands();
        if (n > kMaxOperands) n = kMaxOperands;
        for (unsigned i = 0; i < n; ++i) {
          if (const auto* sub =
                  llvm::dyn_cast<llvm::Constant>(cAfter->getOperand(i))) {
            collectFns(sub, out, depth + 1);
          }
        }
      };

  std::size_t edges_added = 0;

  // Iterate every CCPG node (not edges, because external/declared
  // registration APIs like request_irq don't appear as CCPGEdge::CALL
  // dst — they have no callee body). For each call-site, look directly
  // at the underlying llvm::CallInst to recover the callee's name and
  // its constant function-pointer / ops-struct args.
  std::unordered_set<CCPGNode*> visited;
  for (CCPGEdge* e : ccpg->getEdges()) {
    for (CCPGNode* n : {e->getSrc(), e->getDst()}) {
      if (!n || !visited.insert(n).second) continue;
      if (!n->isCallSite()) continue;
      const llvm::CallInst* ci = n->getLLVMCallInst();
      if (!ci) continue;

      // Resolve callee name. Direct call → getCalledFunction(); indirect
      // call → fall back to the CCPGNode's own name (set by frontend).
      std::string calleeName;
      if (const llvm::Function* called = ci->getCalledFunction()) {
        calleeName = called->getName().str();
      } else if (n->getCPGNode()) {
        calleeName = n->getCPGNode()->getName();
      }
      if (!kRegistrationAPIs.count(calleeName)) continue;

      std::vector<const llvm::Function*> targets;
      visitedConsts.clear();
      for (unsigned i = 0; i < ci->arg_size(); ++i) {
        const llvm::Value* v = ci->getArgOperand(i);
        if (!v) continue;
        const llvm::Value* stripped = v->stripPointerCasts();
        if (!stripped) continue;
        if (const auto* fn = llvm::dyn_cast<llvm::Function>(stripped)) {
          targets.push_back(fn);
        } else if (const auto* gv =
                       llvm::dyn_cast<llvm::GlobalVariable>(stripped)) {
          if (gv->hasInitializer()) {
            collectFns(gv->getInitializer(), targets, 0);
          }
        } else if (const auto* c = llvm::dyn_cast<llvm::Constant>(stripped)) {
          collectFns(c, targets, 0);
        }
      }

      if (targets.empty()) continue;

      std::unordered_set<const llvm::Function*> seen;
      for (const llvm::Function* fn : targets) {
        if (!fn || !seen.insert(fn).second) continue;
        auto it = entryFnToThread.find(fn->getName().str());
        if (it == entryFnToThread.end()) continue;
        CCPGNode* tgtEntry = getThreadEntryNode(it->second);
        if (!tgtEntry) continue;
        addEdge(n, tgtEntry, HBEdgeKind::LIFECYCLE_FLAG,
                "register:" + calleeName + "->" + fn->getName().str());
        ++edges_added;
      }
    }
  }

  // Optional debug breadcrumb (off by default; set HB_LIFECYCLE_DEBUG=1).
  if (envInt("HB_LIFECYCLE_DEBUG", 0) && edges_added > 0) {
    std::printf("[HBGraph] LIFECYCLE_FLAG edges added: %zu\n", edges_added);
  }
}

namespace {
// PROGRAM_ORDER / CALL_RETURN are intra-procedural control flow that both
// concurrent threads traverse when they run the same shared code, so they
// never order two accesses from different threads.
//
// Among the remaining kinds, LOCK_RELEASE_ACQUIRE / FORK_TO_ENTRY /
// JOIN_FROM_EXIT come from lockset analysis and the thread-creation tree,
// i.e. structural facts of the static model. The rest (RCU_SYNC, COMPLETION,
// LIFECYCLE_FLAG, ...) are seeded by recognizing API names; StructuralOnly
// drops them so that those protocols must be established by a recovered
// Order/Wait guarantee instead.
bool isSyncEdge(HBEdgeKind k, HBGraph::SyncPolicy policy) {
  switch (k) {
    case HBEdgeKind::PROGRAM_ORDER:
    case HBEdgeKind::CALL_RETURN:
      return false;
    case HBEdgeKind::LOCK_RELEASE_ACQUIRE:
    case HBEdgeKind::FORK_TO_ENTRY:
    case HBEdgeKind::JOIN_FROM_EXIT:
      return true;
    default:
      return policy == HBGraph::SyncPolicy::AllMechanisms;
  }
}
}  // namespace

bool HBGraph::hbReachable(CCPGNode* n1, CCPGNode* n2, int max_depth,
                          bool requireSyncEdge, SyncPolicy policy) const {
  if (!n1 || !n2) return false;
  if (n1 == n2) return !requireSyncEdge;
  int cap = envInt("HB_MAX_DEPTH", max_depth);

  // BFS state carries whether the path so far crossed a synchronization edge.
  // With requireSyncEdge, n2 only counts as reached once such an edge is on
  // the path. Visited-set is keyed on (node, sawSync) so a node can be
  // re-expanded if we later arrive with sync already crossed.
  std::queue<std::pair<CCPGNode*, int>> q;   // node, depth
  std::unordered_set<CCPGNode*> visNoSync, visSync;
  auto seen = [&](CCPGNode* n, bool sawSync) -> bool {
    auto& set = sawSync ? visSync : visNoSync;
    return !set.insert(n).second;
  };
  // Encode sawSync in a parallel queue to avoid changing the pair type.
  std::queue<bool> qSync;
  q.push({n1, 0});
  qSync.push(false);
  visNoSync.insert(n1);

  while (!q.empty()) {
    CCPGNode* u = q.front().first;
    int d = q.front().second;
    bool sawSync = qSync.front();
    q.pop();
    qSync.pop();
    if (d >= cap) continue;

    auto it = outEdges_.find(u);
    if (it == outEdges_.end()) continue;
    for (const HBEdge& e : it->second) {
      CCPGNode* v = e.dst;
      if (!v) continue;
      bool nextSync = sawSync || isSyncEdge(e.kind, policy);
      if (v == n2 && (!requireSyncEdge || nextSync)) return true;
      if (!seen(v, nextSync)) {
        q.push({v, d + 1});
        qSync.push(nextSync);
      }
    }
  }
  return false;
}

std::vector<HBEdgeKind> HBGraph::classifyPath(CCPGNode* n1, CCPGNode* n2) const {
  std::vector<HBEdgeKind> out;
  if (!n1 || !n2) return out;
  if (n1 == n2) return out;

  int cap = envInt("HB_MAX_DEPTH", 16);

  using Path = std::vector<HBEdgeKind>;
  std::queue<std::pair<CCPGNode*, Path>> q;
  std::unordered_set<CCPGNode*> vis;
  q.push({n1, {}});
  vis.insert(n1);

  while (!q.empty()) {
    CCPGNode* u = q.front().first;
    Path p = std::move(q.front().second);
    q.pop();
    if (static_cast<int>(p.size()) >= cap) continue;

    auto it = outEdges_.find(u);
    if (it == outEdges_.end()) continue;
    for (const HBEdge& e : it->second) {
      CCPGNode* v = e.dst;
      if (!v) continue;
      Path p2 = p;
      p2.push_back(e.kind);
      if (v == n2) {
        return p2;
      }
      if (vis.insert(v).second) {
        q.push({v, std::move(p2)});
      }
    }
  }
  return out;
}

void HBGraph::dumpDot(const std::filesystem::path& outDir) const {
  std::error_code ec;
  std::filesystem::create_directories(outDir, ec);
  std::ofstream f(outDir / "hb-graph.dot");
  if (!f) return;
  f << "digraph HBGraph {\n";
  for (const auto& kv : outEdges_) {
    for (const HBEdge& e : kv.second) {
      f << "  n" << e.src->getId() << " -> n" << e.dst->getId() << " [label=\""
        << kindLabel(e.kind) << "\"];\n";
    }
  }
  f << "}\n";
}
