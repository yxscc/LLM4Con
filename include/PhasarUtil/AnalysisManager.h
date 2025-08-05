// include/Util/AnalysisManager.h

#ifndef ANALYSIS_MANAGER_H
#define ANALYSIS_MANAGER_H

#include <memory>
#include "PhasarUtil/PhasarPointerAnalysis.h" 

class AnalysisManager {
public:
    // 获取单例实例
    static AnalysisManager* getInstance() {
        if (instance == nullptr) {
            instance = new AnalysisManager();
        }
        return instance;
    }

    // 在程序启动时初始化分析器
    void initialize(std::unique_ptr<PhasarPointerAnalysis> analyzer) {
        phasarAnalyzer = std::move(analyzer);
    }

    // 获取分析器接口
    PointerAnalysisInterface* getPointerAnalyzer() {
        // 现在编译器拥有完整的类型信息，知道这是一个合法的转换
        return phasarAnalyzer.get();
    }

    // 禁止拷贝和赋值
    AnalysisManager(const AnalysisManager&) = delete;
    AnalysisManager& operator=(const AnalysisManager&) = delete;

private:
    AnalysisManager() = default;
    ~AnalysisManager() = default; // 析构函数可以放回.h文件中
    static AnalysisManager* instance;

    std::unique_ptr<PhasarPointerAnalysis> phasarAnalyzer;
};

#endif // ANALYSIS_MANAGER_H