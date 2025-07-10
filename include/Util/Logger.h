#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <memory>
#include <mutex>

class Logger {
public:
    static Logger* getInstance();
    void log(const std::string& message);

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    static Logger* instance;
    static std::mutex mutex;
    std::ofstream logFile;
};

#endif // LOGGER_H
