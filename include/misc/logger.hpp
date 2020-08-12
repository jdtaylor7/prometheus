#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>

enum class LogLevel;
class Logger;

enum class LogLevel
{
    error = 0,
    warning = 1,
    info = 2,
};

class Logger
{
public:
    explicit Logger(LogLevel level_) : level(level_) {}

    template <typename T>
    void info(T);
    template <typename T, typename ... Args>
    void info(T, Args...);

    template <typename T>
    void warning(T);
    template <typename T, typename ... Args>
    void warning(T, Args...);

    template <typename T>
    void error(T);
    template <typename T, typename ... Args>
    void error(T, Args...);
private:
    LogLevel level;
    bool first_print = true;

};

template <typename T>
void Logger::info(T first)
{
    if (level >= LogLevel::info)
    {
        if (first_print)
        {
            std::cout << "INFO: ";
            first_print = false;
        }

        std::cout << first;
    }

    // Reset flag.
    first_print = true;
}

template <typename T, typename ... Args>
void Logger::info(T first, Args... args)
{
    if (level >= LogLevel::info)
    {
        if (first_print)
        {
            std::cout << "INFO: ";
            first_print = false;
        }

        std::cout << first;
        info(args...);
    }
}

template <typename T>
void Logger::warning(T first)
{
    if (level >= LogLevel::warning)
    {
        if (first_print)
        {
            std::cout << "WARNING: ";
            first_print = false;
        }

        std::cout << first;

        // Reset flag.
        first_print = true;
    }
}

template <typename T, typename ... Args>
void Logger::warning(T first, Args... args)
{
    if (level >= LogLevel::warning)
    {
        if (first_print)
        {
            std::cout << "WARNING: ";
            first_print = false;
        }

        std::cout << first;
        warning(args...);
    }
}

template <typename T>
void Logger::error(T first)
{
    if (level >= LogLevel::error)
    {
        if (first_print)
        {
            std::cout << "ERROR: ";
            first_print = false;
        }

        std::cout << first;

        // Reset flag.
        first_print = true;
    }
}

template <typename T, typename ... Args>
void Logger::error(T first, Args... args)
{
    if (level >= LogLevel::error)
    {
        if (first_print)
        {
            std::cout << "ERROR: ";
            first_print = false;
        }

        std::cout << first;
        error(args...);
    }
}

Logger logger(LogLevel::warning);

#endif /* LOGGER_HPP */
