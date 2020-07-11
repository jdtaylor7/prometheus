#ifndef COM_PORT_HPP
#define COM_PORT_HPP

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
#elif OS_LINUX
#include "libusb.h"
#endif

#include "bounded_buffer.hpp"

enum class PortState : uint8_t
{
    Running,
    Stopped,
};

class ComPort
{
public:
    ComPort(std::size_t packet_len_,
            char packet_start_symbol_,
            char packet_stop_symbol);

    ~ComPort();

    ComPort(const ComPort&) = delete;
    ComPort& operator=(const ComPort&) = delete;

    ComPort(ComPort&&) = delete;
    ComPort& operator=(ComPort&&) = delete;

    bool init();

    bool start();
    void stop();

    std::vector<unsigned int> find_ports();
    bool connect(unsigned int);
    bool auto_connect();
    void disconnect();

    bool is_valid() const;
    bool is_connected() const { return connected; }
    unsigned int get_connected_port() const { return connected_port; }
    std::vector<unsigned int> get_available_ports() const { return available_ports; }
    bool is_reading() const { return running_state == PortState::Running; }

    std::shared_ptr<std::string> get_latest_packet();
    std::size_t get_buffer_size() const { return buffer->size(); };
    void clear_buffer() { buffer->clear(); };

#ifdef OS_CYGWIN
    void invalidate_handle(HANDLE&);
#elif OS_LINUX
    void invalidate_handle();
#endif

    static unsigned async_receive(void*);
private:

#ifdef OS_CYGWIN
    HANDLE handle;
    HANDLE thread_started;
    HANDLE thread_term;
#elif OS_LINUX
#endif
    const std::size_t COM_BEG = 2;
    const std::size_t COM_END = 10;

    std::thread thread_handle;
    bool initialized = false;
    bool connected = false;
    unsigned int connected_port = 0;
    std::vector<unsigned int> available_ports{};

    const std::size_t packet_len;
    const char packet_start_symbol;
    const char packet_stop_symbol;
    std::shared_ptr<BoundedBuffer<char>> buffer =
        std::make_shared<BoundedBuffer<char>>((packet_len * 2) - 1);

    bool build_new_packet = true;
    std::string latest_packet{};
public:
    PortState running_state = PortState::Stopped;
    std::mutex running_state_m;
};

#endif /* COM_PORT_HPP */
