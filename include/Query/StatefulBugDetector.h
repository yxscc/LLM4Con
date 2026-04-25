// in include/Query/StatefulBugDetector.h

#pragma once

#include "LLMUtil/ThreadPair.h"
#include "Query/HypothesisVerifier.h"
#include <vector>
#include <string>
#include <sstream>
#include <filesystem>

namespace llm_client {
    class VerificationAgent;
}

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

    const llm_client::StatefulRule& getRule() const { return rule; }
    const std::vector<std::pair<std::string, CCPGNode*>>& getPath() const { return path; }
    const llm_client::ThreadPair& getThreadPair() const { return threads; }

private:
    llm_client::StatefulRule rule;
    std::vector<std::pair<std::string, CCPGNode*>> path;
    const llm_client::ThreadPair& threads;
};

// Simple bug description for external sources (e.g., lazy-init race from API discovery)
struct ExternalBug {
    std::string bugType;      // e.g., "LAZY_INIT_RACE"
    std::string functionName;
    std::string description;
    std::string sharedVariable;
    std::string evidence;
    
    std::string toString() const {
        std::stringstream ss;
        ss << "========== " << bugType << " Detected ==========\n";
        ss << "Function: " << functionName << "\n";
        if (!sharedVariable.empty())
            ss << "Shared Variable: " << sharedVariable << "\n";
        ss << "Description: " << description << "\n";
        if (!evidence.empty())
            ss << "Evidence: " << evidence << "\n";
        ss << "==========================================================";
        return ss.str();
    }
};

// 新的检测器类，专门用于检测状态协议违规
class StatefulBugDetector {
public:
    StatefulBugDetector() = default;

    // 核心检测方法，接收AgentManager的分析结果
    void detect(
        const std::vector<llm_client::ThreadPair>& threadPairs,
        const std::set<const llvm::Value*>& candidateSharedObjects,
        llm_client::VerificationAgent* verificationAgent = nullptr
    );

    // 添加来自外部来源的bug（如API发现阶段的lazy-init竞争）
    void addExternalBug(const ExternalBug& bug) {
        externalBugs.push_back(bug);
    }

    // Open-hypothesis mode: hypotheses already verified by HypothesisVerifier
    void detectFromHypotheses(const std::vector<Hypothesis>& hypotheses, CCPG* ccpg);

    void printResults(const fs::path& outputDir) const;
    
    size_t getTotalBugCount() const {
        return detectedBugs.size() + externalBugs.size() + confirmedHypotheses_.size();
    }

private:
    std::vector<StatefulBug> detectedBugs;
    std::vector<ExternalBug> externalBugs;
    std::vector<Hypothesis> confirmedHypotheses_;
    CCPG* hypothesisCcpg_ = nullptr;
    std::set<std::pair<std::string, NodeLoc>> reported_bugs_locations;
};

} // namespace query