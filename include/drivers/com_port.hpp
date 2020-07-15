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
    ComPort(std::size_t packet_len_,
            char packet_start_symbol_,
            char packet_stop_symbol);

    ~ComPort();

    // Disallow copying and moving.
    ComPort(const ComPort&) = delete;
    ComPort& operator=(const ComPort&) = delete;
    ComPort(ComPort&&) = delete;
    ComPort& operator=(ComPort&&) = delete;

    bool open(const std::string&);
    void close();
    bool auto_open();
    bool config();

    bool start_reading();
    void stop_reading();

    std::vector<std::string> find_ports();

    bool is_open() const { return port_open; }
    std::string get_port_name() const { return port_name; }
    std::vector<std::string> get_available_ports() const { return available_ports; }
    bool is_reading() const { return port_reading.load(); }

    std::shared_ptr<std::string> get_latest_packet();
    std::size_t get_buffer_size() const { return buffer->size(); };

    static unsigned async_receive(void*);
private:
    const std::string COM_PORT_PREFIX = "\\\\.\\COM";
    const std::size_t COM_BEG = 2;
    const std::size_t COM_END = 10;

#ifdef OS_CYGWIN
    HANDLE handle;
    HANDLE thread_started;
    HANDLE thread_term;
#endif

    std::thread thread_handle;

    bool port_configured = false;
    bool port_open = false;
    std::string port_name{};
    std::atomic<bool> port_reading = false;

    std::vector<std::string> available_ports{};

    const std::size_t packet_len;
    const char packet_start_symbol;
    const char packet_stop_symbol;
    std::shared_ptr<BoundedBuffer<char>> buffer =
        std::make_shared<BoundedBuffer<char>>((packet_len * 2) - 1);

    bool build_new_packet = true;
    std::string latest_packet{};
};

// #endif /* OS_CYGWIN */  // TODO uncomment

#endif /* COM_PORT_HPP */
