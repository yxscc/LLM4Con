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

  bool hbReachable(CCPGNode* n1, CCPGNode* n2, int max_depth = 16) const;

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
