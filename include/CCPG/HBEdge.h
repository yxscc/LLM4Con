// HBEdge.h — granular happens-before edge kinds (M7 §4.1)
#ifndef HB_EDGE_H
#define HB_EDGE_H

#include <string>

class CCPGNode;

namespace ccpg {

// HB 边的来源类别；供调试 / DSL eval 使用。
enum class HBEdgeKind {
  PROGRAM_ORDER,       // §3.1 同函数 CFG 顺序
  CALL_RETURN,         // §3.1 caller→callee；callee→return
  LOCK_RELEASE_ACQUIRE, // §3.2 release(L) → acquire(L)
  FORK_TO_ENTRY,       // fork → 子线程入口
  JOIN_FROM_EXIT,      // §3.6 子线程 return → join 之后
  RCU_SYNC,
  REFCOUNT_LAST_PUT,
  REL_ACQ_PAIR,
  BH_IRQ_INTERVAL,
  COMPLETION,   // §3.6 complete → wait_for_completion 之后
  LIFECYCLE_FLAG,
};

struct HBEdge {
  CCPGNode* src;
  CCPGNode* dst;
  HBEdgeKind kind;
  std::string evidence;
};

} // namespace ccpg

#endif
