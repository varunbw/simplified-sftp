#ifndef PERF_SSFTP
#define PERF_SSFTP

#include <chrono>

// -- Performance Counters
typedef std::chrono::_V2::system_clock::time_point TimePoint;

// Used to determine what format to print, in TimeElapsed()
enum TimePrecision {
    MICROSECONDS = 1,
    MILLISECONDS = 2,
    SECONDS = 3
};

/* 
    @brief Gives the current system time in microseconds
    @return Current system time in microseconds
*/
TimePoint TimeNow();

/* 
    @brief Calculates the time difference between `start` and current time
    @param `start` Start of the interval
    @param `precision` Whether to print microseconds, milliseconds, or seconds elapsed since start (default: MILLISECONDS)
    @param `division` What to divide the result of end-start by, useful for calculating average time in loops (default: 1)
    @return String of format "`Time elapsed: {time} {unit}`\n"

    NOTE: If you want to pass a division parameter, you also need to pass a TimePrecision parameter, you cant skip it
*/
std::string TimeElapsed(TimePoint start, TimePrecision precision = TimePrecision::MILLISECONDS, int division = 1);

/* 
    @brief Calculates the time difference between `start` and current time
    @param `start` Start of the interval
    @param `end` End of the interval
    @param `precision` Whether to print microseconds, milliseconds, or seconds elapsed since start (default: MILLISECONDS)
    @param `division` What to divide the result of end-start by, useful for calculating average time in loops (default: 1)
    @return String of format "`Time elapsed: {time} {unit}`\n"

    NOTE: If you want  to pass a division parameter, you also need to pass a TimePrecision parameter, you cant skip it
*/
std::string TimeElapsed(TimePoint start, TimePoint end, TimePrecision precision = TimePrecision::MILLISECONDS, int division = 1);

#endif