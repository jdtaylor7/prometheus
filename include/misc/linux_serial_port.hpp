#ifndef LINUX_SERIAL_PORT
#define LINUX_SERIAL_PORT

#ifdef OS_LINUX

#include <atomic>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "libserial/SerialStream.h"

#include "bounded_buffer.hpp"

class LinuxSerialPort
{
public:
    LinuxSerialPort(std::shared_ptr<BoundedBuffer<char>>);

    LinuxSerialPort(const LinuxSerialPort&) = delete;
    LinuxSerialPort& operator=(const LinuxSerialPort&) = delete;

    LinuxSerialPort(LinuxSerialPort&&) = delete;
    LinuxSerialPort& operator=(LinuxSerialPort&&) = delete;

    bool open(const std::string&);
    void config(
        const LibSerial::BaudRate,
        const LibSerial::CharacterSize,
        const LibSerial::FlowControl,
        const LibSerial::Parity,
        const LibSerial::StopBits);

    bool is_data_available();
    bool is_reading() const;

    void start_reading();
    void stop_reading();
private:
    LibSerial::SerialStream stream;
    std::string serial_port;

    std::shared_ptr<BoundedBuffer<char>> byte_buffer;
    std::atomic<bool> keep_reading = false;
};

#endif /* OS_LINUX */

#endif /* LINUX_SERIAL_PORT */
