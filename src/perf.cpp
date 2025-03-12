#include <chrono>
#include <stdexcept>
#include <string>

#include "../include/perf.hpp"
#include "../include/util.hpp"

/* 
    @brief Gives the current system time in microseconds
    @return Current system time in microseconds
*/
TimePoint TimeNow() {
    return std::chrono::high_resolution_clock::now();
}

/* 
    @brief Calculates the time difference between `start` and current time
    @param `start` Start of the interval
    @param `end` End of the interval
    @param `precision` Whether to print microseconds, milliseconds, or seconds elapsed since start (default: MILLISECONDS)
    @param `division` What to divide the result of end-start by, useful for calculating average time in loops (default: 1)
    @return String of format "`Time elapsed: {time} {unit}`\n"

    NOTE: If you want  to pass a division parameter, you also need to pass a TimePrecision parameter, you cant skip it
*/
std::string TimeElapsed(TimePoint start, TimePoint end, TimePrecision precision, int division) {

    if (division == 0) 
        throw std::invalid_argument(
            RED_START +
            std::string("[ERROR]: Attempted to divide by 0 due to invalid parameter in TimeElapsed(TimePoint, TimePrecision, int)") +
            RESET_COLOR
        );

    std::string res;
    if (precision ==  TimePrecision::MICROSECONDS) {
        std::chrono::duration<double, std::micro> duration = end - start;
        double elapsed = duration.count() / division;
        res = std::string(std::to_string(elapsed) + " us");
    }
    
    else if (precision ==  TimePrecision::MILLISECONDS) {
        std::chrono::duration<double, std::milli> duration = end - start;
        double elapsed = duration.count() / division;
        res = std::string(std::to_string(elapsed) + " ms");
    }

    else if (precision ==  TimePrecision::SECONDS) {
        std::chrono::duration<double> duration = end - start;
        double elapsed = duration.count() / division;
        res = std::string(std::to_string(elapsed) + " s");
    }

    else {
        throw std::invalid_argument(
            RED_START +
            std::string("[ERROR]: Invalid precision parameter passed to TimeElapsed(TimePoint, TimePrecision, int)") +
            RESET_COLOR
        );
    }

    return res;
}