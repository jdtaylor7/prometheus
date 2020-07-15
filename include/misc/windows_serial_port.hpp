#ifndef WINDOWS_SERIAL_PORT_HPP
#define WINDOWS_SERIAL_PORT_HPP

#ifdef OS_CYGWIN

#include <atomic>

#include <Process.h>
#include <windows.h>

#include "bounded_buffer.hpp"

class WindowsSerialPort
{
public:
    WindowsSerialPort(std::size_t packet_len_,
        char packet_start_symbol_,
        char packet_stop_symbol_);

    ~WindowsSerialPort();

    WindowsSerialPort(const WindowsSerialPort&) = delete;
    WindowsSerialPort operator=(const WindowsSerialPort&) = delete;

    WindowsSerialPort(WindowsSerialPort&&) = delete;
    WindowsSerialPort& operator=(WindowsSerialPort&&) = delete;

    std::vector<std::string> find_ports() const;
    bool is_open() const;
    bool is_reading() const;

    bool open(const std::string&);
    bool config();

    void start_reading();
    void stop_reading();

private:
    std::shared_ptr<BoundedBuffer<char>> byte_buffer;
    std::atomic<bool> keep_reading = false;
};

#endif /* OS_CYGWIN */

#endif /* WINDOWS_SERIAL_PORT_HPP */
