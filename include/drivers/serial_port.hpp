#ifndef SERIAL_PORT
#define SERIAL_PORT

#include <memory>

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
public:
#ifdef OS_CYGWIN
    explicit SerialPort(std::shared_ptr<BoundedBuffer<char>>);
#elif OS_LINUX
    SerialPort(
        std::shared_ptr<BoundedBuffer<char>>,
        LinuxSerialPortConfig);
#endif
    ~SerialPort();

    // Disallow copying and moving.
    SerialPort(const SerialPort&) = delete;
    SerialPort operator=(const SerialPort&) = delete;
    SerialPort(SerialPort&&) = delete;
    SerialPort& operator=(SerialPort&&) = delete;

    std::vector<std::string> find_ports();

    bool open(const std::string&);
    bool config();

    bool start_reading();
    void stop_reading();

    bool is_open() const;
    bool is_reading() const;
    std::string get_port_name() const;
    std::vector<std::string> get_available_ports() const;

private:
#ifdef OS_CYGWIN
    WindowsSerialPort windows_port;
#elif OS_LINUX
    LinuxSerialPort linux_port;
#endif
};

#endif /* SERIAL_PORT */
