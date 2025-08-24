#include "Util/Logger.h"
#include "Util/TargetPath.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>

Logger* Logger::instance = nullptr;
std::mutex Logger::mutex;

// Helper to get current timestamp
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    return ss.str();
}

Logger::Logger() {
    fs::path log_path = TargetPath::getInstance()->getOutputDir() / "llm_conversations.log";
    logFile.open(log_path, std::ios_base::app);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " << log_path << std::endl;
    }
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

Logger* Logger::getInstance() {
    std::lock_guard<std::mutex> lock(mutex);
    if (instance == nullptr) {
        instance = new Logger();
    }
    return instance;
}

void Logger::log(const std::string& message) {
    if (logFile.is_open()) {
        logFile << "[" << getCurrentTimestamp() << "] " << message << std::endl;
    }
}
