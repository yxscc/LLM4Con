#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "LLMUtil/LLMClient.h"
#include "LLMUtil/Conversation.h"
#include "CCPG/ThreadAPIUtil.h"

namespace llm_client {

/**
 * ThreadAPIDiscoveryAgent
 * 
 * 使用LLM自动发现项目中对标准线程/同步API的封装函数。
 * 这使得工具能够处理使用自定义封装（如HTTrack的hts_newthread）的项目，
 * 而不仅仅是直接调用pthread API的项目。
 */
class ThreadAPIDiscoveryAgent {
public:
    struct DiscoveredAPI {
        std::string functionName;
        ThreadAPIUtil::TYPE apiType;
        std::string wrappedAPI;      // 被封装的标准API
        std::string confidence;       // high/medium/low
        std::string evidence;         // 发现的证据（代码片段）
    };

    // Lazy-init race condition detected in lock wrappers
    struct LazyInitRace {
        std::string functionName;     // The wrapper function with the bug
        std::string sharedVariable;   // The mutex pointer being raced on
        std::string checkStatement;   // The if-check code
        std::string initStatement;    // The initialization code
        std::string evidence;         // Explanation
    };

    explicit ThreadAPIDiscoveryAgent(std::shared_ptr<LLMClient> client);
    
    /**
     * 扫描源代码目录，发现线程/锁相关的封装函数
     * @param sourceDir 源代码目录路径
     * @return 发现的封装函数列表
     */
    std::vector<DiscoveredAPI> discoverWrapperAPIs(const std::string& sourceDir);
    
    /**
     * 将发现的API注册到ThreadAPIUtil中
     * @param discoveredAPIs 发现的API列表
     */
    void registerDiscoveredAPIs(const std::vector<DiscoveredAPI>& discoveredAPIs);
    
    /**
     * 一站式方法：发现并注册所有封装API
     * @param sourceDir 源代码目录路径
     * @return 发现并注册的API数量
     */
    int discoverAndRegister(const std::string& sourceDir);

    /**
     * 获取检测到的lazy-init竞争
     */
    const std::vector<LazyInitRace>& getDetectedLazyInitRaces() const { return detectedLazyInitRaces; }

private:
    std::vector<LazyInitRace> detectedLazyInitRaces;
    std::shared_ptr<LLMClient> llmClient;
    
    /**
     * 收集源代码中可能包含线程/锁操作的文件内容
     */
    std::string collectRelevantSourceCode(const std::string& sourceDir);
    
    /**
     * 构建发现封装函数的提示词
     */
    std::string buildDiscoveryPrompt(const std::string& sourceCode);
    
    /**
     * 解析LLM响应，提取发现的API
     */
    std::vector<DiscoveredAPI> parseDiscoveryResponse(const std::string& response);
    
    /**
     * 将API类型字符串转换为枚举
     */
    ThreadAPIUtil::TYPE stringToAPIType(const std::string& typeStr);
};

} // namespace llm_client
