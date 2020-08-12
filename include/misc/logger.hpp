#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>

enum class LogLevel
{
    fatal = 0,
    error,
    warning,
    info,
    debug,
};

class Logger
{
public:
    explicit Logger(LogLevel threshold_) : threshold(threshold_) {}

    template <typename T>
    void log(LogLevel, T);
    template <typename T, typename ... Args>
    void log(LogLevel, T, Args...);
private:
    LogLevel threshold;
    bool first_print = true;
};

template <typename T>
void Logger::log(LogLevel level, T first)
{
    if (level <= threshold)
    {
        if (first_print)
        {
            switch (level)
            {
                case LogLevel::fatal: std::cout << "FATAL: "; break;
                case LogLevel::error: std::cerr << "ERROR: "; break;
                case LogLevel::warning: std::cerr << "WARNING: "; break;
                case LogLevel::info: std::cout << "INFO: "; break;
                case LogLevel::debug: std::cout << "DEBUG: "; break;
            }
            first_print = false;
        }

        std::cout << first;

        // Reset flag.
        first_print = true;
    }
}

template <typename T, typename ... Args>
void Logger::log(LogLevel level, T first, Args... args)
{
    if (level <= threshold)
    {
        if (first_print)
        {
            switch (level)
            {
                case LogLevel::fatal: std::cout << "FATAL: "; break;
                case LogLevel::error: std::cerr << "ERROR: "; break;
                case LogLevel::warning: std::cerr << "WARNING: "; break;
                case LogLevel::info: std::cout << "INFO: "; break;
                case LogLevel::debug: std::cout << "DEBUG: "; break;
            }
            first_print = false;
        }

        std::cout << first;
        log(level, args...);
    }
}

extern Logger logger;

#endif /* LOGGER_HPP */
