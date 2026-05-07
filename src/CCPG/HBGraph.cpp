#include "CCPG/HBGraph.h"

#include <cstdlib>
#include <fstream>
#include <queue>
#include <unordered_set>

#include "CCPG/AliasChecker.h"
#include "CCPG/CCPG.h"
#include "CCPG/LSAnalysis.h"
#include "CCPG/ThreadCreationTree.h"
#include "CCPG/ThreadAPIUtil.h"

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

void HBGraph::buildRCUEdges(CCPG*) {}
void HBGraph::buildRefcountEdges(CCPG*) {}
void HBGraph::buildContextIntervalEdges(CCPG*) {}
void HBGraph::buildLifecycleFlagEdges(CCPG*) {}

bool HBGraph::hbReachable(CCPGNode* n1, CCPGNode* n2, int max_depth) const {
  if (!n1 || !n2) return false;
  if (n1 == n2) return true;
  int cap = envInt("HB_MAX_DEPTH", max_depth);

  std::queue<std::pair<CCPGNode*, int>> q;
  std::unordered_set<CCPGNode*> vis;
  q.push({n1, 0});
  vis.insert(n1);

  while (!q.empty()) {
    CCPGNode* u = q.front().first;
    int d = q.front().second;
    q.pop();
    if (d >= cap) continue;

    auto it = outEdges_.find(u);
    if (it == outEdges_.end()) continue;
    for (const HBEdge& e : it->second) {
      CCPGNode* v = e.dst;
      if (!v) continue;
      if (v == n2) return true;
      if (vis.insert(v).second) {
        q.push({v, d + 1});
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
