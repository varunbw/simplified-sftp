#include <iostream>
#include <string>
#include <format>

#include "../include/logger.hpp"

namespace Log {
    void Error(const std::string& functionName, const std::string& message) {
        std::cerr << std::format(
            RED_START "[ERROR] {}: {}\n" RESET_COLOR,
            functionName, message
        );
    }

    void Success(const std::string& functionName, const std::string& message) {
        std::cout << std::format(
            GREEN_START "[SUCCESS] {}: {}\n" RESET_COLOR,
            functionName, message
        );
    }

    void Info(const std::string& functionName, const std::string& message) {
        std::cout << std::format(
            "{}: {}\n",
            functionName, message
        );
    }

    void Warning(const std::string& functionName, const std::string& message) {
        std::cout << std::format(
            YELLOW_START "[WARNING] {}: {}\n" RESET_COLOR,
            functionName, message
        );
    }
}