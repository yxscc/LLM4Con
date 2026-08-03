// HBGraph.h — M7 happens-before 图（singleton，在 LSAnalysis::build 之后构建）
#ifndef HB_GRAPH_H
#define HB_GRAPH_H

#include <cstddef>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "CCPG/HBEdge.h"

class CCPG;
class CCPGNode;

class HBGraph {
public:
  static HBGraph* getInstance();

  // 一次性构建；调用顺序：CCPG::build() → LSAnalysis::build() → build(ccpg)
  void build(CCPG* ccpg);

  // Which edge kinds count as cross-thread synchronization for a
  // requireSyncEdge query.
  //   AllMechanisms  -- every non-PROGRAM_ORDER/CALL_RETURN kind.
  //   StructuralOnly -- only orderings derived from lockset analysis and the
  //                     thread-creation tree (LOCK_RELEASE_ACQUIRE,
  //                     FORK_TO_ENTRY, JOIN_FROM_EXIT). Protocol-level
  //                     orderings seeded from API-name tables (RCU_SYNC,
  //                     COMPLETION, LIFECYCLE_FLAG) are excluded: those
  //                     mechanisms are recovered as Order/Wait guarantees and
  //                     must reach the checker through the contract, not
  //                     through a built-in API classification.
  enum class SyncPolicy { AllMechanisms, StructuralOnly };

  // When requireSyncEdge is true, a path from n1 to n2 only establishes
  // happens-before if it traverses at least one edge that `policy` accepts as
  // genuine cross-thread synchronization. A path made purely of PROGRAM_ORDER
  // + CALL_RETURN edges is shared intra-procedural control flow through code
  // that BOTH concurrent threads execute -- it does NOT order two accesses
  // from different threads. Used by the concurrency / no-hb (UAF) predicates
  // so shared kernel helpers no longer spuriously "order" a real cross-thread
  // race out of existence.
  bool hbReachable(CCPGNode* n1, CCPGNode* n2, int max_depth = 16,
                   bool requireSyncEdge = false,
                   SyncPolicy policy = SyncPolicy::AllMechanisms) const;

  void dumpDot(const std::filesystem::path& outDir) const;

  // 返回 n1→n2 的某条路径上的边类型（若不可达则空）
  std::vector<ccpg::HBEdgeKind> classifyPath(CCPGNode* n1, CCPGNode* n2) const;

private:
  HBGraph() = default;

  void clear();

  void buildProgramOrderEdges(CCPG* ccpg);
  void buildLockEdges(CCPG* ccpg);
  void buildForkJoinEdges(CCPG* ccpg);
  void buildCompletionEdges(CCPG* ccpg);

  void addEdge(CCPGNode* src, CCPGNode* dst, ccpg::HBEdgeKind kind,
               const std::string& evidence = "");

  // Phase A 完整版桩（MVP 不调用）
  void buildRCUEdges(CCPG* ccpg);
  void buildRefcountEdges(CCPG* ccpg);
  void buildContextIntervalEdges(CCPG* ccpg);
  void buildLifecycleFlagEdges(CCPG* ccpg);

  std::unordered_map<CCPGNode*, std::vector<ccpg::HBEdge>> outEdges_;
  std::unordered_map<CCPGNode*, std::vector<ccpg::HBEdge>> inEdges_;
  static HBGraph* instance;
};

#endif
