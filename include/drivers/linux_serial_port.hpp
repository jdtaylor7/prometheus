#ifndef LINUX_SERIAL_PORT
#define LINUX_SERIAL_PORT

#ifdef OS_LINUX

#include <atomic>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "libserial/SerialStream.h"

#include "bounded_buffer.hpp"

struct LinuxSerialPortConfig
{
    LinuxSerialPortConfig(
            LibSerial::BaudRate br_,
            LibSerial::CharacterSize cs_,
            LibSerial::FlowControl fc_,
            LibSerial::Parity py_,
            LibSerial::StopBits sb_) :
        br(br_), cs(cs_), fc(fc_), py(py_), sb(sb_) {}

    LibSerial::BaudRate br;
    LibSerial::CharacterSize cs;
    LibSerial::FlowControl fc;
    LibSerial::Parity py;
    LibSerial::StopBits sb;
};

/*
 * Linux implementation of a serial port. Uses libserial library. Operates
 * synchronously.
 */
class LinuxSerialPort
{
public:
    LinuxSerialPort(std::shared_ptr<BoundedBuffer<char>>);
    ~LinuxSerialPort();

    // Disallow copying and moving.
    LinuxSerialPort(const LinuxSerialPort&) = delete;
    LinuxSerialPort& operator=(const LinuxSerialPort&) = delete;
    LinuxSerialPort(LinuxSerialPort&&) = delete;
    LinuxSerialPort& operator=(LinuxSerialPort&&) = delete;

    bool open(const std::string&);
    void config(const LinuxSerialPortConfig&);

    std::vector<std::string> find_ports() const;  // TODO implement

    bool is_open() const;
    bool is_reading() const;
    std::string get_port_name() const;  // TODO implement

    void start_reading();
    void stop_reading();
private:
    LibSerial::SerialStream stream;
    std::string port_name{};

    std::shared_ptr<BoundedBuffer<char>> byte_buffer;
    bool port_open = false;
    std::atomic<bool> port_reading = false;
};

#endif /* OS_LINUX */

#endif /* LINUX_SERIAL_PORT */
