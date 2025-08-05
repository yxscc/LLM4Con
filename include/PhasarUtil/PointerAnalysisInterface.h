#pragma once

#include <set>
#include <vector>
#include <string>

namespace llvm {
    class Value;
    class Function;
    class Instruction;
}

struct EntryPointInfo {
    std::string functionName;
    std::string fileName;
    unsigned int lineNumber;
};

// 定义一个纯虚类（接口）
class PointerAnalysisInterface {
public:
    virtual ~PointerAnalysisInterface() = default;

    /**
     * @brief 查询两个LLVM Value是否可能是别名（即指向同一块内存）。
     * @param V1 第一个LLVM Value。
     * @param V2 第二个LLVM Value。
     * @return 如果它们可能是别名，则返回true。
     */
    virtual bool areAliases(const llvm::Value *V1, const llvm::Value *V2) = 0;

    /**
     * @brief 获取一个指针的指向集。
     * @param Ptr 指向指针的LLVM Value。
     * @return 一个包含所有可能内存位置的集合。
     */
    virtual std::set<const llvm::Value *> getPointsToSet(const llvm::Value *Ptr) = 0;

    /**
     * @brief (辅助函数) 根据函数名和变量名，在LLVM IR中查找对应的Value。
     * 这是连接源代码信息和IR信息的桥梁。
     * @param funcName 变量所在的函数名。
     * @param varName 变量名。
     * @return 指向该变量的LLVM Value，如果找不到则返回nullptr。
     */
    virtual const llvm::Value* getValueByName(const std::string &funcName, const std::string &varName) = 0;

    /**
     * @brief 获取潜在的程序入口点.
     * @return 一个包含入口点信息的向量.
     */
    virtual std::vector<EntryPointInfo> getPotentialEntryPoints() = 0;

    /**
     * @brief (新方法) 获取分析引擎加载的所有LLVM Function。
     * @return 包含所有 llvm::Function 指针的向量。
     */
    virtual std::vector<const llvm::Function *> getAllLLVMFunctions() const = 0;

    /**
     * @brief 获取在给定调用点可能被调用的所有函数。
     * 这是一个高级抽象，隐藏了ICFG或CallGraph等实现细节。
     * @param callInst 一个指向调用指令的指针。
     * @return 一个包含所有潜在被调用函数指针的向量。
     */
    virtual std::vector<const llvm::Function*> getCalleesOfCallAt(const llvm::Instruction* callInst) const = 0;
};