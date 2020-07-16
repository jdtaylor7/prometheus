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
    LinuxSerialPort(
        std::shared_ptr<BoundedBuffer<char>>,
        LinuxSerialPortConfig);
    ~LinuxSerialPort();

    // Disallow copying and moving.
    LinuxSerialPort(const LinuxSerialPort&) = delete;
    LinuxSerialPort& operator=(const LinuxSerialPort&) = delete;
    LinuxSerialPort(LinuxSerialPort&&) = delete;
    LinuxSerialPort& operator=(LinuxSerialPort&&) = delete;

    std::vector<std::string> find_ports();  // TODO implement

    bool open(const std::string&);
    bool config();

    bool start_reading();
    void stop_reading();

    bool is_open() const { return port_open; }
    bool is_reading() const { return port_reading.load(); }
    std::string get_port_name() const { return port_name; }
    std::vector<std::string> get_available_ports() const { return available_ports; }
private:
    /*
     * Linux-specific state.
     */
    LinuxSerialPortConfig cfg;
    LibSerial::SerialStream stream;

    /*
     * General serial port state.
     */
    std::shared_ptr<BoundedBuffer<char>> buffer;

    bool port_open = false;
    bool port_configured = false;
    std::atomic<bool> port_reading = false;

    std::string port_name{};  // TODO actually assign somewhere
    std::vector<std::string> available_ports{};  // actually assign somewhere
};

#endif /* OS_LINUX */

#endif /* LINUX_SERIAL_PORT */
