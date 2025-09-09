// in include/Query/StatefulBugDetector.h

#pragma once

#include "LLMUtil/ThreadPair.h"
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

class CCPGNode;

namespace query {

// 用于存储一个被发现的状态协议违规的详细信息
class StatefulBug {
public:
    StatefulBug(
        const llm_client::StatefulRule& violated_rule,
        const std::vector<std::pair<std::string, CCPGNode*>>& violation_path,
        const llm_client::ThreadPair& thread_pair
    );

    std::string toString() const;

private:
    llm_client::StatefulRule rule;
    std::vector<std::pair<std::string, CCPGNode*>> path;
    const llm_client::ThreadPair& threads;
};

// 新的检测器类，专门用于检测状态协议违规
class StatefulBugDetector {
public:
    StatefulBugDetector() = default;

    // 核心检测方法，接收AgentManager的分析结果
    void detect(
        const std::vector<llm_client::ThreadPair>& threadPairs,
        const std::set<const llvm::Value*>& candidateSharedObjects
    );

    // 将检测到的漏洞打印到文件
    void printResults(const fs::path& outputDir) const;

private:
    std::vector<StatefulBug> detectedBugs;
    std::set<std::pair<std::string, NodeLoc>> reported_bugs_locations;
};

} // namespace query