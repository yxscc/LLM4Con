#include "LLMUtil/ThreadAPIDiscoveryAgent.h"
#include "Util/Logger.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>
#include <iostream>
#include <algorithm>

namespace fs = std::filesystem;

namespace llm_client {

ThreadAPIDiscoveryAgent::ThreadAPIDiscoveryAgent(std::shared_ptr<LLMClient> client)
    : llmClient(client) {}

std::string ThreadAPIDiscoveryAgent::collectRelevantSourceCode(const std::string& sourceDir) {
    std::stringstream collectedCode;
    
    // 关键词用于筛选可能包含线程/锁操作的文件
    std::vector<std::string> relevantKeywords = {
        "thread", "mutex", "lock", "pthread", "fork", "spawn",
        "synchron", "atomic", "semaphore", "barrier", "condition",
        "env", "port", "platform"  // Often contain thread-related code
    };
    
    // 遍历源代码目录
    for (const auto& entry : fs::recursive_directory_iterator(sourceDir)) {
        if (!entry.is_regular_file()) continue;
        
        std::string ext = entry.path().extension().string();
        if (ext != ".c" && ext != ".h" && ext != ".cpp" && ext != ".hpp") continue;
        
        std::string filename = entry.path().filename().string();
        std::string filenameLower = filename;
        std::transform(filenameLower.begin(), filenameLower.end(), filenameLower.begin(), ::tolower);
        
        // 检查文件名是否包含相关关键词
        bool isRelevant = false;
        for (const auto& kw : relevantKeywords) {
            if (filenameLower.find(kw) != std::string::npos) {
                isRelevant = true;
                break;
            }
        }
        
        // 如果文件名不相关，检查文件内容
        if (!isRelevant) {
            std::ifstream file(entry.path());
            std::string content((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
            std::string contentLower = content;
            std::transform(contentLower.begin(), contentLower.end(), contentLower.begin(), ::tolower);
            
            // 检查是否包含线程/同步相关调用
            if (contentLower.find("pthread_create") != std::string::npos ||
                contentLower.find("pthread_mutex") != std::string::npos ||
                contentLower.find("pthread_cond") != std::string::npos ||
                contentLower.find("std::thread") != std::string::npos ||
                contentLower.find("std::mutex") != std::string::npos ||
                contentLower.find("std::condition_variable") != std::string::npos ||
                contentLower.find("createthread") != std::string::npos ||
                contentLower.find("_beginthreadex") != std::string::npos) {
                isRelevant = true;
            }
        }
        
        if (isRelevant) {
            std::ifstream file(entry.path());
            std::string content((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
            
            // 限制每个文件的大小
            if (content.size() > 15000) {
                content = content.substr(0, 15000) + "\n... [truncated] ...";
            }
            
            collectedCode << "// ========== File: " << entry.path().filename().string() << " ==========\n";
            collectedCode << content << "\n\n";
        }
    }
    
    std::string result = collectedCode.str();
    
    // 总体大小限制
    if (result.size() > 100000) {
        result = result.substr(0, 100000) + "\n... [truncated due to size limit] ...";
    }
    
    return result;
}

std::string ThreadAPIDiscoveryAgent::buildDiscoveryPrompt(const std::string& sourceCode) {
    std::stringstream prompt;
    
    prompt << R"(# Task: Discover Thread/Lock API Wrapper Functions

You are analyzing a C/C++ codebase to identify **wrapper functions** that encapsulate standard POSIX thread and synchronization APIs.

## Standard APIs to Look For

### Thread Creation (FORK)
- `pthread_create`, `_beginthreadex`, `CreateThread`
- `std::thread` constructor (C++11)

### Thread Join (JOIN)  
- `pthread_join`, `WaitForSingleObject`
- `std::thread::join()` (C++11)

### Mutex Lock (ACQUIRE)
- `pthread_mutex_lock`, `pthread_rwlock_rdlock`, `pthread_rwlock_wrlock`, `EnterCriticalSection`, `WaitForSingleObject`

### Mutex Unlock (RELEASE)
- `pthread_mutex_unlock`, `pthread_rwlock_unlock`, `LeaveCriticalSection`, `ReleaseMutex`

### Mutex Init (MUTEX_INI)
- `pthread_mutex_init`, `InitializeCriticalSection`

### Mutex Destroy (MUTEX_DESTROY)
- `pthread_mutex_destroy`, `DeleteCriticalSection`

### Condition Variable (COND_WAIT, COND_SIGNAL, COND_BROADCAST)
- `pthread_cond_wait`, `pthread_cond_signal`, `pthread_cond_broadcast`

## Your Task

1. Analyze the provided source code
2. Find functions that **wrap** the above standard APIs
3. A wrapper function typically:
   - Has a different name than the standard API
   - Internally calls one of the standard APIs
   - May add additional logic (error handling, logging, lazy initialization, etc.)

## Output Format

For each discovered wrapper function, output in this exact format:

```
WRAPPER: <function_name>
TYPE: <FORK|JOIN|ACQUIRE|RELEASE|MUTEX_INI|MUTEX_DESTROY|COND_WAIT|COND_SIGNAL|COND_BROADCAST>
WRAPS: <standard_api_name>
CONFIDENCE: <HIGH|MEDIUM|LOW>
EVIDENCE: <brief code snippet or explanation>
---
```

If no wrapper functions are found, output:
```
NO_WRAPPERS_FOUND
```

## Important Notes

- Focus on **custom wrapper functions**, not direct calls to standard APIs
- Pay special attention to functions with names containing: thread, mutex, lock, sync, etc.

## Source Code to Analyze

)";

    prompt << "```c\n" << sourceCode << "\n```\n";
    
    return prompt.str();
}

std::vector<ThreadAPIDiscoveryAgent::DiscoveredAPI> 
ThreadAPIDiscoveryAgent::parseDiscoveryResponse(const std::string& response) {
    std::vector<DiscoveredAPI> result;
    
    if (response.find("NO_WRAPPERS_FOUND") != std::string::npos) {
        return result;
    }
    
    // 解析每个WRAPPER块
    std::regex wrapperRegex(R"(WRAPPER:\s*(\w+)\s*\nTYPE:\s*(\w+)\s*\nWRAPS:\s*(\w+)\s*\nCONFIDENCE:\s*(\w+)\s*\nEVIDENCE:\s*([^\n-]+))", 
                           std::regex::multiline);
    
    std::sregex_iterator iter(response.begin(), response.end(), wrapperRegex);
    std::sregex_iterator end;
    
    while (iter != end) {
        std::smatch match = *iter;
        if (match.size() >= 6) {
            DiscoveredAPI api;
            api.functionName = match[1].str();
            api.apiType = stringToAPIType(match[2].str());
            api.wrappedAPI = match[3].str();
            api.confidence = match[4].str();
            api.evidence = match[5].str();
            
            // 只添加有效的API类型
            if (api.apiType != ThreadAPIUtil::TYPE::DUMMY) {
                result.push_back(api);
            }
        }
        ++iter;
    }
    
    // 如果正则没有匹配到，尝试更宽松的解析
    if (result.empty()) {
        // 尝试匹配简单的模式
        std::regex simpleRegex(R"(WRAPPER:\s*(\w+)[\s\S]*?TYPE:\s*(\w+))", std::regex::multiline);
        std::sregex_iterator simpleIter(response.begin(), response.end(), simpleRegex);
        
        while (simpleIter != end) {
            std::smatch match = *simpleIter;
            if (match.size() >= 3) {
                DiscoveredAPI api;
                api.functionName = match[1].str();
                api.apiType = stringToAPIType(match[2].str());
                api.confidence = "MEDIUM";
                
                if (api.apiType != ThreadAPIUtil::TYPE::DUMMY) {
                    result.push_back(api);
                }
            }
            ++simpleIter;
        }
    }
    
    // [DISABLED] Lazy-init race detection moved to normal stateful bug detection flow
    // std::regex lazyInitRegex(R"(LAZY_INIT_RACE_DETECTED:\s*(\w+)\s*\n(?:SHARED_VARIABLE:\s*([^\n]+)\s*\n)?(?:CHECK_STATEMENT:\s*([^\n]+)\s*\n)?(?:INIT_STATEMENT:\s*([^\n]+)\s*\n)?(?:EVIDENCE:\s*([^\n-]+))?)", 
    //                          std::regex::multiline);
    // ... parsing disabled ...
    
    return result;
}

ThreadAPIUtil::TYPE ThreadAPIDiscoveryAgent::stringToAPIType(const std::string& typeStr) {
    std::string upperType = typeStr;
    std::transform(upperType.begin(), upperType.end(), upperType.begin(), ::toupper);
    
    if (upperType == "FORK" || upperType == "THREAD_CREATE") return ThreadAPIUtil::TYPE::FORK;
    if (upperType == "JOIN" || upperType == "THREAD_JOIN") return ThreadAPIUtil::TYPE::JOIN;
    if (upperType == "ACQUIRE" || upperType == "LOCK" || upperType == "MUTEX_LOCK") return ThreadAPIUtil::TYPE::ACQUIRE;
    if (upperType == "RELEASE" || upperType == "UNLOCK" || upperType == "MUTEX_UNLOCK") return ThreadAPIUtil::TYPE::RELEASE;
    if (upperType == "MUTEX_INI" || upperType == "MUTEX_INIT") return ThreadAPIUtil::TYPE::MUTEX_INI;
    if (upperType == "MUTEX_DESTROY") return ThreadAPIUtil::TYPE::MUTEX_DESTROY;
    if (upperType == "COND_WAIT") return ThreadAPIUtil::TYPE::COND_WAIT;
    if (upperType == "COND_SIGNAL") return ThreadAPIUtil::TYPE::COND_SIGNAL;
    if (upperType == "COND_BROADCAST") return ThreadAPIUtil::TYPE::COND_BROADCAST;
    if (upperType == "DETACH") return ThreadAPIUtil::TYPE::DETACH;
    if (upperType == "EXIT") return ThreadAPIUtil::TYPE::EXIT;
    if (upperType == "TRY_ACQUIRE" || upperType == "TRYLOCK") return ThreadAPIUtil::TYPE::TRY_ACQUIRE;
    
    return ThreadAPIUtil::TYPE::DUMMY;
}

std::vector<ThreadAPIDiscoveryAgent::DiscoveredAPI> 
ThreadAPIDiscoveryAgent::discoverWrapperAPIs(const std::string& sourceDir) {
    std::cout << "\n[Thread API Discovery] Scanning source code for wrapper functions..." << std::endl;
    
    // 1. 收集相关源代码
    std::string sourceCode = collectRelevantSourceCode(sourceDir);
    
    if (sourceCode.empty()) {
        std::cout << "[Thread API Discovery] No relevant source files found." << std::endl;
        return {};
    }
    
    std::cout << "[Thread API Discovery] Collected " << sourceCode.size() << " bytes of relevant code." << std::endl;
    
    // 2. 构建提示词并调用LLM
    std::string prompt = buildDiscoveryPrompt(sourceCode);
    
    try {
        Conversation convo(llmClient, "You are an expert C/C++ code analyzer specializing in concurrent programming patterns.");
        std::string response = convo.send_message(prompt);
        
        // 3. 解析响应
        auto discoveredAPIs = parseDiscoveryResponse(response);
        
        std::cout << "[Thread API Discovery] Discovered " << discoveredAPIs.size() << " wrapper function(s)." << std::endl;
        
        for (const auto& api : discoveredAPIs) {
            std::cout << "  - " << api.functionName << " -> " 
                      << ThreadAPIUtil::getTypeString(api.apiType) 
                      << " (confidence: " << api.confidence << ")" << std::endl;
        }
        
        // [DISABLED] Lazy-init race reporting - now handled by normal stateful bug detection
        // if (!detectedLazyInitRaces.empty()) { ... }
        
        return discoveredAPIs;
        
    } catch (const std::exception& e) {
        std::cerr << "[Thread API Discovery] Error during LLM analysis: " << e.what() << std::endl;
        return {};
    }
}

void ThreadAPIDiscoveryAgent::registerDiscoveredAPIs(const std::vector<DiscoveredAPI>& discoveredAPIs) {
    auto* apiUtil = ThreadAPIUtil::getInstance();
    
    for (const auto& api : discoveredAPIs) {
        // 只注册高/中置信度的API
        if (api.confidence == "HIGH" || api.confidence == "MEDIUM" || api.confidence == "high" || api.confidence == "medium") {
            apiUtil->addDynamicAPI(api.functionName, api.apiType);
            std::cout << "[Thread API Discovery] Registered: " << api.functionName 
                      << " as " << ThreadAPIUtil::getTypeString(api.apiType) << std::endl;
        }
    }
}

int ThreadAPIDiscoveryAgent::discoverAndRegister(const std::string& sourceDir) {
    auto discoveredAPIs = discoverWrapperAPIs(sourceDir);
    
    if (discoveredAPIs.empty()) {
        std::cout << "[Thread API Discovery] No wrapper functions discovered." << std::endl;
        return 0;
    }
    
    registerDiscoveredAPIs(discoveredAPIs);
    
    return discoveredAPIs.size();
}

} // namespace llm_client
