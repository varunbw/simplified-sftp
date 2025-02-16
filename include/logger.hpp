#ifndef LOGGER_SSFTP
#define LOGGER_SSFTP

#include <iostream>
#include <format>

#define RED_START "\033[1;31m"
#define GREEN_START "\033[1;32m"
#define YELLOW_START "\033[1;33m"
#define RESET_COLOR "\033[0m"

namespace Log {
    void Error(const std::string& functionName, const std::string& message);
    void Success(const std::string& functionName, const std::string& message);
    void Info(const std::string& functionName, const std::string& message);
    void Warning(const std::string& functionName, const std::string& message);
}

#endif