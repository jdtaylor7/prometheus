#ifndef SERIAL_PORT
#define SERIAL_PORT

#ifdef OS_CYGWIN
#include <Process.h>
#include <windows.h>
#elif OS_LINUX
#include "libusb.h"
#endif

#include "linux_serial_port.hpp"
#include "windows_serial_port.hpp"

class SerialPort
{

};

#endif /* SERIAL_PORT */
