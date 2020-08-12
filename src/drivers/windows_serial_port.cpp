#include "windows_serial_port.hpp"

#ifdef OS_CYGWIN

explicit WindowsSerialPort::WindowsSerialPort(std::shared_ptr<BoundedBuffer<char>> buffer_)
    : buffer{buffer_}
{
}

WindowsSerialPort::~WindowsSerialPort()
{
    CloseHandle(handle);
}

std::vector<std::string> WindowsSerialPort::find_ports()
{
    available_ports.clear();

    for (std::size_t i = COM_BEG; i < COM_END; i++)
    {
        std::string com_port_str = COM_PORT_PREFIX + std::to_string(i);

        logger.log(LogLevel::info: "Checking COM", i, "...\n");
        handle = CreateFile(com_port_str.c_str(),  // filename
                            GENERIC_READ,          // access method
                            0,                     // cannot share
                            NULL,                  // no security attributes
                            OPEN_EXISTING,         // file action, value for serial ports
                            0,                     // FILE_FLAG_OVERLAPPED TODO change
                            NULL);                 // ignored

        if (handle != INVALID_HANDLE_VALUE)
        {
            logger.log(LogLevel::info: "COM", i, " available\n");
            available_ports.push_back(i);
        }
        CloseHandle(handle);
    }
    return available_ports;
}

bool WindowsSerialPort::open(const std::string& port)
{
    if (port_open)
    {
        logger.log(LogLevel::info: "Port is already opened\n");
        return false;
    }
    std::string com_port_str = COM_PORT_PREFIX + port;

    handle = CreateFile(com_port_str.c_str(),  // filename
                        GENERIC_READ,          // access method
                        0,                     // cannot share
                        NULL,                  // no security attributes
                        OPEN_EXISTING,         // file action, value for serial ports
                        0,                     // FILE_FLAG_OVERLAPPED TODO change
                        NULL);                 // ignored

    if (handle != INVALID_HANDLE_VALUE)
    {
        logger.log(LogLevel::info: "Successfully opened ", com_port_str, '\n');
        port_open = true;
        port_name = com_port_str;
        return true;
    }
    return false;
}

void WindowsSerialPort::close()
{
}

bool WindowsSerialPort::auto_open()
{
    find_ports();
    if (available_ports.size() == 1)
    {
        logger.log(LogLevel::info: "Automatically opening ", available_ports[0], '\n');
        open(available_ports[0]);
        config();
        return true;
    }
    else if (available_ports.size() > 1)
    {
        logger.log(LogLevel::info: "Not auto-opening any ports: More than 1 available\n");
        return false;
    }
    else
    {
        logger.log(LogLevel::info: "Not auto-opening any ports: None available\n");
        return false;
    }
}

bool WindowsSerialPort::config()
{
    if (!port_open)
    {
        logger.log(LogLevel::warning: "Cannot configure port before opening\n");
        return false;
    }
    if (port_configured)
    {
        logger.log(LogLevel::warning: "Port has already been configured\n");
        return false;
    }

    /*
     * Set COM mask. Register event for receive buffer getting data.
     */
    if (!SetCommMask(handle, EV_RXCHAR))
    {
        logger.log(LogLevel::error: "Cannot set COM mask: ", GetLastError(), '\n');
        return false;
    }

    /*
     * Set com port paramaters.
     */
    DCB dcb = {0};
    dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(handle, &dcb))
    {
        logger.log(LogLevel::error: "Cannot get comm state: ", GetLastError(), '\n');
        return false;
    }

    dcb.BaudRate = 9600;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;

    if (!SetCommState(handle, &dcb))
    {
        logger.log(LogLevel::error: "Cannot set comm state: ", GetLastError(), '\n');
        return false;
    }

    /*
     * Set timeouts.
     */
    COMMTIMEOUTS com_timeouts;
    com_timeouts.ReadIntervalTimeout = MAXDWORD;
    com_timeouts.ReadTotalTimeoutMultiplier = 0;
    com_timeouts.ReadTotalTimeoutConstant = 0;

    if (!SetCommTimeouts(handle, &com_timeouts))
    {
        logger.log(LogLevel::error: "Cannot set comm timeouts: ", GetLastError(), '\n');
        return false;
    }

    port_configured = true;
    return true;
}

bool WindowsSerialPort::start_reading()
{
    if (is_reading())
    {
        logger.log(LogLevel::warning: "Already reading from port\n");
        return false;
    }
    if (!port_open)
    {
        logger.log(LogLevel::warning: "Cannot read from port before opening it\n");
        return false;
    }
    if (!port_configured)
    {
        logger.log(LogLevel::warning: "Must configure port before reading from it\n");
        return false;
    }

    port_reading.store(true);
    buffer->clear();

    /*
     * Create thread.
     */
    thread_term = CreateEvent(NULL,   // No security attributes
                              false,  // No manual reset
                              false,  // starts nonsignaled
                              NULL);  // no name
    thread_started = CreateEvent(NULL,   // No security attributes
                                 false,  // No manual reset
                                 false,  // starts nonsignaled
                                 NULL);  // no name

    thread_handle = std::thread(&WindowsSerialPort::async_receive, (void*)this);
    thread_handle.detach();

    DWORD wait_rv = WaitForSingleObject(thread_started, INFINITE);

    assert(wait_rv == WAIT_OBJECT_0);

    CloseHandle(thread_started);
    handle = INVALID_HANDLE_VALUE;

    return true;
}

void WindowsSerialPort::stop_reading()
{
    this->port_reading.store(false);
    SetEvent(thread_term);
}

unsigned WindowsSerialPort::async_receive(void* params)
{
    WindowsSerialPort* com_ptr = static_cast<WindowsSerialPort*>(params);
    bool keep_processing = true;
    DWORD event_mask = 0;

    OVERLAPPED ov;
    memset(&ov, 0, sizeof(ov));
    ov.hEvent = CreateEvent(NULL,   // event can't be inherited
                            true,   // manual reset
                            false,  // initially nonsignaled
                            NULL);  // no name
    HANDLE handles[2];
    handles[0] = com_ptr->thread_term;

    DWORD wait_rv;
    SetEvent(com_ptr->thread_started);

    while (keep_processing)
    {
        if (!WaitCommEvent(com_ptr->handle, &event_mask, &ov))
        {
            logger.log(LogLevel::info: "Could not set comm mask: ", GetLastError(), '\n');
            assert(GetLastError() == ERROR_IO_PENDING);
        }

        handles[1] = ov.hEvent;
        wait_rv = WaitForMultipleObjects(2,          // # object handles
                                         handles,    // array of object handles
                                         FALSE,      // return when any object is signaled
                                         INFINITE);  // no timeout

        switch (wait_rv)
        {
        case WAIT_OBJECT_0:
            {
                return 0;
            }
        case WAIT_OBJECT_0 + 1:
            {
                try
                {
                    BOOL read_rv = false;
                    DWORD bytes_read = 0;

                    std::string tmp_str{};

                    OVERLAPPED ov_read;
                    memset(&ov_read, 0, sizeof(ov_read));
                    ov_read.hEvent = CreateEvent(NULL,   // no security attributes
                                                 true,   // manual reset
                                                 false,  // initially nonsignaled
                                                 NULL);  // no name

                    do
                    {
                        ResetEvent(ov_read.hEvent);

                        char tmp_buf[1];
                        int i_size = sizeof(tmp_buf);
                        memset(tmp_buf, 0, sizeof(tmp_buf));

                        read_rv = ReadFile(com_ptr->handle,  // device handle
                                           tmp_buf,          // receiving buffer
                                           sizeof(tmp_buf),  // max # bytes to read
                                           &bytes_read,      // # bytes read
                                           &ov_read);        // OVERLAPPED struct ptr
                        if (!read_rv)
                        {
                            keep_processing = false;
                            break;
                        }
                        if (bytes_read > 0)
                        {
                            com_ptr->buffer->force_push(tmp_buf[0]);
                        }
                    } while (bytes_read > 0);
                    CloseHandle(ov_read.hEvent);
                }
                catch(...)
                {
                    logger.log(LogLevel::error: "Failed to read from serial thread\n");
                    assert(0);
                }
                ResetEvent(ov.hEvent);
                break;
            }
        }
    }
    return 0;
}

#endif /* OS_CYGWIN */
