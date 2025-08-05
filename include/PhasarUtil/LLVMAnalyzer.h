#ifndef LLVM_ANALYZER_H
#define LLVM_ANALYZER_H

#include <cxxabi.h>
#include <regex>
#include <filesystem>
#include <unordered_set>
#include <set>
#include <fstream>
#include <tuple>


class LLVMAnalyzer {
public:
    static LLVMAnalyzer* getInstance() {
        if (instance == nullptr) {
            instance = new LLVMAnalyzer();
        }
        return instance;
    }

    int getLineNumberFromSourceLoc(std::string sourceLoc);

    std::string getFileFromSourceLoc(std::string sourceLoc);

    std::string demangle(const char* mangledName);

    std::string demangle_valueName(const char* mangledName);


private:

    static LLVMAnalyzer* instance;  // 静态实例指针

    LLVMAnalyzer() { }  // 私有构造函数
    ~LLVMAnalyzer() {}  // 私有析构函数
    LLVMAnalyzer(const LLVMAnalyzer&) = delete;            // 阻止复制构造
    LLVMAnalyzer& operator=(const LLVMAnalyzer&) = delete; // 阻止赋值操作

};

#endif