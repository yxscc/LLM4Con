#ifndef CONCURRENCY_ANALYSIS_HELPER_H
#define CONCURRENCY_ANALYSIS_HELPER_H

#include "LLMUtil/ThreadPair.h"

namespace query {

/**
 * @brief 一个通用的并发访问对遍历器.
 *
 * 这个函数封装了数据竞争检测中的核心循环逻辑。它会遍历一个线程对中
 * 所有共享同一个内存对象（由 representative 标识）的并发内存访问对。
 *
 * @tparam BugDetectorFunc 一个可调用对象 (例如 lambda)，其签名为:
 * void(const MemoryAccess& acc1,
 * const LLM::ConcurrencyContract& contract1,
 * const MemoryAccess& acc2,
 * const LLM::ConcurrencyContract& contract2)
 * @param pair 包含两个线程及其预先计算好的内存访问图的线程对.
 * @param check_function 当找到一对冲突的并发访问时要执行的检测逻辑.
 */
template <typename BugDetectorFunc>
void forEachConcurrentAccessPair(
    const llm_client::ThreadPair& pair,
    BugDetectorFunc check_function
) {
    // 确保我们只在实际并发的线程对上工作
    if (!pair.analysis.actually_concurrent) {
        return;
    }

    const auto& accessMap1 = pair.analysis.accessMap1;
    const auto& accessMap2 = pair.analysis.accessMap2;

    const auto& contract1 = pair.contract1;
    const auto& contract2 = pair.contract2;

    // 遍历第一个线程的内存访问图
    for (const auto& [representative, access_list1] : accessMap1) {
        // 在第二个线程的图中查找同一个内存对象
        auto it = accessMap2.find(representative);
        if (it != accessMap2.end()) {
            const auto& access_list2 = it->second;
            
            // 对两个访问列表进行两两比较
            for (const auto& acc1 : access_list1) {
                for (const auto& acc2 : access_list2) {
                    // 调用用户提供的检测函数来执行具体的规则检查
                    check_function(acc1, contract1, acc2, contract2);
                }
            }
        }
    }
}

} // namespace query

#endif // CONCURRENCY_ANALYSIS_HELPER_H