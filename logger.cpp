#include "logger.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>

namespace Logger {

    void log(Level level, const std::string& message) {
        const char* levelStr = "UNKNOWN";
        switch (level) {
        case Level::Info:
            levelStr = "INFO";
            break;
        case Level::Warning:
            levelStr = "WARN";
            break;
        case Level::Error:
            levelStr = "ERROR";
            break;
        default:
            break;
        }

        std::time_t now = std::time(nullptr);
        std::tm localTime{};
        localtime_s(&localTime, &now);

        char timeBuffer[32]{};
        std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &localTime);

        std::ostringstream oss;
        oss << "[" << timeBuffer << "][" << levelStr << "] " << message;
        const std::string formatted = oss.str();

        std::cout << formatted << std::endl;

        std::ofstream logFile("LunoCheatChecker.log", std::ios::app);
        if (logFile.is_open()) {
            logFile << formatted << std::endl;
        }
    }

    void info(const std::string& message) {
        log(Level::Info, message);
    }

    void warning(const std::string& message) {
        log(Level::Warning, message);
    }

    void error(const std::string& message) {
        log(Level::Error, message);
    }

} // namespace Logger

