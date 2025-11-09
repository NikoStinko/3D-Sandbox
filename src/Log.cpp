#include "Log.h"
#include <chrono>
#include <ctime>
#include <filesystem>

std::mutex ComponentLogger::globalMutex;

static std::string timestamp()
{
    using namespace std::chrono;
    auto now = system_clock::now();
    std::time_t t = system_clock::to_time_t(now);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    return std::string(buf);
}

ComponentLogger::ComponentLogger(const std::string &componentName) : component(componentName)
{
    try {
        std::filesystem::create_directories("logs");
        std::string path = "logs/" + component + ".log";
        stream.open(path, std::ios::out | std::ios::app);
    } catch (...) {
        // Fallback: do nothing if logs can't be opened
    }
}

ComponentLogger::~ComponentLogger()
{
    if (stream.is_open()) stream.flush();
}

void ComponentLogger::write(const char *level, const std::string &msg)
{
    std::lock_guard<std::mutex> lock(globalMutex);
    if (stream.is_open()) {
        stream << "[" << timestamp() << "]" << " [" << level << "] " << msg << '\n';
        stream.flush();
    }
}

void ComponentLogger::info(const std::string &msg)
{
    write("INFO", msg);
}

void ComponentLogger::error(const std::string &msg)
{
    write("ERROR", msg);
}

void ComponentLogger::debug(const std::string &msg)
{
    write("DEBUG", msg);
}
