#ifndef COM_PORT_HPP
#define COM_PORT_HPP

// #ifdef OS_CYGWIN  // TODO uncomment

#include <atomic>
#include <cassert>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#ifdef OS_CYGWIN
#include <Process.h>
#include <windows.h>
#endif

#include "bounded_buffer.hpp"

class ComPort
{
public:
    ComPort(std::shared_ptr<BoundedBuffer<char>>);
    ~ComPort();

    // Disallow copying and moving.
    ComPort(const ComPort&) = delete;
    ComPort& operator=(const ComPort&) = delete;
    ComPort(ComPort&&) = delete;
    ComPort& operator=(ComPort&&) = delete;

    std::vector<std::string> find_ports();

    bool open(const std::string&);
    void close();
    bool auto_open();
    bool config();

    bool start_reading();
    void stop_reading();

    bool is_open() const { return port_open; }
    bool is_reading() const { return port_reading.load(); }
    std::string get_port_name() const { return port_name; }
    std::vector<std::string> get_available_ports() const { return available_ports; }
private:
    /*
     * Windows-specific state.
     */
    const std::string COM_PORT_PREFIX = "\\\\.\\COM";
    const std::size_t COM_BEG = 2;
    const std::size_t COM_END = 10;

#ifdef OS_CYGWIN
    HANDLE handle;
    HANDLE thread_started;
    HANDLE thread_term;
#endif
    std::thread thread_handle;
    static unsigned async_receive(void*);  // TODO make private

    /*
     * General serial port state.
     */
    std::shared_ptr<BoundedBuffer<char>> buffer;

    bool port_open = false;
    bool port_configured = false;
    std::atomic<bool> port_reading = false;

    std::string port_name{};
    std::vector<std::string> available_ports{};
};

// #endif /* OS_CYGWIN */  // TODO uncomment

#endif /* COM_PORT_HPP */
