#include "windows_serial_port.hpp"

#ifdef OS_CYGWIN

WindowsSerialPort::WindowsSerialPort()
{

}

WindowsSerialPort::~WindowsSerialPort()
{
    CloseHandle(handle);
}

void WindowsSerialPort::stop_reading()
{
    keep_reading.store(false);
}

#endif /* OS_CYGWIN */
