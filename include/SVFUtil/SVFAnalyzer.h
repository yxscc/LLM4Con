#include <cxxabi.h>
#include <regex>
#include <filesystem>
#include <unordered_set>
#include <set>
#include <fstream>
#include <tuple>


class SVFAnalyzer {
public:
    static SVFAnalyzer* getInstance() {
        if (instance == nullptr) {
            instance = new SVFAnalyzer();
        }
        return instance;
    }

    int getLineNumberFromSourceLoc(std::string sourceLoc);

    std::string getFileFromSourceLoc(std::string sourceLoc);

    std::string demangle(const char* mangledName);

    std::string demangle_valueName(const char* mangledName);


private:

    static SVFAnalyzer* instance;  // 静态实例指针

    SVFAnalyzer() { }  // 私有构造函数
    ~SVFAnalyzer() {}  // 私有析构函数
    SVFAnalyzer(const SVFAnalyzer&) = delete;            // 阻止复制构造
    SVFAnalyzer& operator=(const SVFAnalyzer&) = delete; // 阻止赋值操作

};