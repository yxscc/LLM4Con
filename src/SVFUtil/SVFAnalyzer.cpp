// SVFAnalyzer.cpp

#include "SVFUtil/SVFAnalyzer.h"

// 初始化静态成员变量为 nullptr
SVFAnalyzer* SVFAnalyzer::instance = nullptr;

int SVFAnalyzer::getLineNumberFromSourceLoc(std::string sourceLoc){
    if(sourceLoc.empty()){
        return -1;
    }
    std::smatch match;
    std::regex lnPattern(R"("ln"\s*:\s*(\d+))");

    std::string lineNumber;

    if (std::regex_search(sourceLoc, match, lnPattern) && match.size() > 1) {
        lineNumber = match.str(1);
        return std::stoi(lineNumber);
    }

    return -1;
}

std::string SVFAnalyzer::getFileFromSourceLoc(std::string sourceLoc){
    if(sourceLoc.empty()){
        return "";
    }
    std::smatch match;
    std::regex filePattern(R"(\"(file|fl)\"\s*:\s*\"([^"]+)\")");

    std::string fileName;

    if (std::regex_search(sourceLoc, match, filePattern) && match.size() > 1) {
        fileName = match.str(2);
        return fileName;
    }
    return "";
}

std::string SVFAnalyzer::demangle(const char* mangledName) {
    int status;
    std::unique_ptr<char, void(*)(void*)> demangledName(
        abi::__cxa_demangle(mangledName, nullptr, nullptr, &status),
        std::free
    );
    if (status != 0) {
        return mangledName; // demangling failed
    }

    std::string result = demangledName.get();
    size_t firstParenthesis = result.find('(');
    if (firstParenthesis != std::string::npos) {
        result = result.substr(0, firstParenthesis);
    }

    return result;
}

std::string SVFAnalyzer::demangle_valueName(const char* mangledName) {
    int status;
    std::unique_ptr<char, void(*)(void*)> demangledName(
        abi::__cxa_demangle(mangledName, nullptr, nullptr, &status),
        std::free
    );

    if (status != 0) {
        return mangledName; // demangling failed
    }

    std::string result = demangledName.get();

    // 处理局部变量名（如 _ZZ4oncePvE4lock）
    size_t lastColon = result.find_last_of("::");
    if (lastColon != std::string::npos) {
        // 提取变量名（如 "lock"）
        result = result.substr(lastColon + 1);
    }

    // 去除函数参数部分（如 "once(void*)" -> "once"）
    size_t firstParenthesis = result.find('(');
    if (firstParenthesis != std::string::npos) {
        result = result.substr(0, firstParenthesis);
    }

    return result;
}


