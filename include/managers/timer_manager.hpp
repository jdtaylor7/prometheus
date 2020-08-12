#ifndef TIMER_MANAGER_HPP
#define TIMER_MANAGER_HPP

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <unordered_map>

#include "logger.hpp"

/*
 * Valid timer names.
 */
enum class TimerName : std::size_t
{
    ComScanTimer    = 0,
    ComConnectTimer = 1,
    ComReadTimer    = 2,
};

/*
 * Individual timer object.
 */
class Timer
{
public:
    explicit Timer(std::chrono::duration<double, std::milli> timeout_) :
        timeout(timeout_) {}

    bool is_finished() const;

    bool start();
    void stop();
private:
    std::chrono::duration<double, std::milli> timeout;

    std::atomic<bool> cancel = false;
    std::atomic<bool> done = true;
};

bool Timer::is_finished() const
{
    return done.load();
}

bool Timer::start()
{
    if (done.load())
    {
        done.store(false);

        std::thread start_thread([this]{
            using namespace std::chrono_literals;

            auto start_time = std::chrono::steady_clock::now();
            auto current_time = std::chrono::steady_clock::now();

            while (1)
            {
                if (cancel.load())
                {
                    cancel.store(false);
                    return;
                }

                std::this_thread::sleep_for(1ms);
                current_time = std::chrono::steady_clock::now();
                if ((current_time - start_time) > timeout)
                {
                    done.store(true);
                    break;
                }
            }
        });
        start_thread.detach();
        return true;
    }
    else
    {
        return false;
    }
}

void Timer::stop()
{
    cancel.store(true);
}

/*
 * Manages multiple timers.
 */
class TimerManager
{
public:
    bool is_finished(TimerName) const;

    bool register_timer(TimerName, std::chrono::duration<double, std::milli>);
    bool start_timer(TimerName);
    void stop_timer(TimerName);
private:
    std::unordered_map<TimerName, Timer> timers;
};

bool TimerManager::is_finished(TimerName timer_name) const
{
    return timers.at(timer_name).is_finished();
}

bool TimerManager::register_timer(TimerName timer_name, std::chrono::duration<double, std::milli> timeout)
{
    auto search = timers.find(timer_name);
    if (search != std::end(timers))
    {
        logger.log(LogLevel::error, "TimerManager::register_timer: Timer with that name already exists\n");
        return false;
    }
    else
    {
        timers.emplace(timer_name, timeout);
        return true;
    }
}

bool TimerManager::start_timer(TimerName timer_name)
{
    return timers.at(timer_name).start();
}

void TimerManager::stop_timer(TimerName timer_name)
{
    timers.at(timer_name).stop();
}

#endif /* TIMER_MANAGER_HPP */
