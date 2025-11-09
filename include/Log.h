#ifndef LOG_H
#define LOG_H

#include <string>
#include <fstream>
#include <mutex>

class ComponentLogger {
public:
    explicit ComponentLogger(const std::string &componentName);
    ~ComponentLogger();

    void info(const std::string &msg);
    void error(const std::string &msg);
    void debug(const std::string &msg);

private:
    std::string component;
    std::ofstream stream;
    static std::mutex globalMutex;
    void write(const char *level, const std::string &msg);
};

#endif
