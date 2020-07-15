#ifndef SERIAL_PORT
#define SERIAL_PORT

#ifdef OS_CYGWIN
#include <Process.h>
#include <windows.h>
#elif OS_LINUX
#include "libusb.h"
#endif

#ifdef OS_CYGWIN
#include "windows_serial_port.hpp"
#elif OS_LINUX
#include "linux_serial_port.hpp"
#endif

/*
 * Abstraction class representing cross-platform serial port. Currently works
 * for Cygwin and Linux platforms.
 */
class SerialPort
{
    SerialPort(std::shared_ptr<BoundedBuffer<char>>);
    ~SerialPort();

    SerialPort(const SerialPort&) = delete;
    SerialPort operator=(const SerialPort&) = delete;

    SerialPort(SerialPort&&) = delete;
    SerialPort& operator=(SerialPort&&) = delete;

    std::vector<std::string> find_ports() const;
    bool is_open() const;
    bool is_reading() const;
    std::string get_port_name() const;

    bool open(const std::string&);
#ifdef OS_CYGWIN
    void config();
#elif OS_LINUX
    void config(const LinuxSerialPortConfig&);
#endif

    void start_reading();
    void stop_reading();

private:
#ifdef OS_CYGWIN
    WindowsSerialPort windows_port;
#elif OS_LINUX
    LinuxSerialPort linux_port;
#endif
};

#endif /* SERIAL_PORT */
