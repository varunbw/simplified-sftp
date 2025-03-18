#ifndef LOGGER_SSFTP
#define LOGGER_SSFTP

#include "utils.hpp"

namespace Log {
    void Error(const std::string& functionName, const std::string& message);
    void Success(const std::string& functionName, const std::string& message);
    void Info(const std::string& functionName, const std::string& message);
    void Warning(const std::string& functionName, const std::string& message);
}

#endif