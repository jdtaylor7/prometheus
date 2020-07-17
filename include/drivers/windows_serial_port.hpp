#ifndef WINDOWS_SERIAL_PORT_HPP
#define WINDOWS_SERIAL_PORT_HPP

#ifdef OS_CYGWIN

#include <atomic>
#include <cassert>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <Process.h>
#include <windows.h>

#include "bounded_buffer.hpp"

class WindowsSerialPort
{
public:
    explicit WindowsSerialPort(std::shared_ptr<BoundedBuffer<char>>);
    ~WindowsSerialPort();

    // Disallow copying and moving.
    WindowsSerialPort(const WindowsSerialPort&) = delete;
    WindowsSerialPort& operator=(const WindowsSerialPort&) = delete;
    WindowsSerialPort(WindowsSerialPort&&) = delete;
    WindowsSerialPort& operator=(WindowsSerialPort&&) = delete;

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

    HANDLE handle;
    HANDLE thread_started;
    HANDLE thread_term;

    std::thread thread_handle;
    static unsigned async_receive(void*);

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

#endif /* OS_CYGWIN */

#endif /* WINDOWS_SERIAL_PORT_HPP */
