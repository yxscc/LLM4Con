#include <iostream>
#include <chrono>
#include <string>
#include <unordered_map>
#include <stack>

class ExecutionTimer {
private:
    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> startTimes;
    std::unordered_map<std::string, double> elapsedTimes;

    ExecutionTimer() {}
    static ExecutionTimer * instance;

public:

    static ExecutionTimer * getInstance() {
        if (instance == nullptr) {
            instance = new ExecutionTimer();
        }
        return instance;
    }
    
    // 开始计时
    void start(const std::string& processName) {
        std::cout << "[START] " << processName << " ..." << std::endl;
        std::cout.flush();  // 确保立即输出
        startTimes[processName] = std::chrono::high_resolution_clock::now();
    }

    // 结束计时
    void stop(const std::string& processName) {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto startTime = startTimes[processName];
        std::chrono::duration<double> elapsed = endTime - startTime;
        elapsedTimes[processName] = elapsed.count();
        std::cout << processName << " succeed! " << std::endl;
    }

    // 获取某个过程的执行时间
    double getElapsedTime(const std::string& processName) const {
        if (elapsedTimes.find(processName) != elapsedTimes.end()) {
            return elapsedTimes.at(processName);
        }
        return 0.0;  // 如果未找到，返回 0
    }

    // 打印所有过程的执行时间
    void printAllTimes(fs::path outputDir) const {
        std::ofstream file(outputDir / "execution-time.txt");
        // 倒序输出，确保先输出的先打印
        std::stack<std::string> processNames = std::stack<std::string>();
        for (const auto& [processName, time] : elapsedTimes) {
            processNames.push(processName);
        }
        while (!processNames.empty()) {
            std::string processName = processNames.top();
            processNames.pop();
            std::cout << processName << " elapsed time: " << elapsedTimes.at(processName) << "s" << std::endl;
            file << processName << " elapsed time: " << elapsedTimes.at(processName) << "s" << std::endl;
        }
        file.close();
    }
};