#include "com_port.hpp"

#include <array>
#include <algorithm>

ComPort::ComPort(std::shared_ptr<BoundedBuffer<char>> buffer_) : buffer(buffer_)
{
}

ComPort::~ComPort()
{
#ifdef OS_CYGWIN
    CloseHandle(handle);
#endif
}

bool ComPort::config()
{
    if (!port_open)
    {
        std::cout << "Cannot configure port before opening.\n";
        return false;
    }

    if (port_configured)
    {
        std::cout << "Port has already been configured.\n";
        return false;
    }

#ifdef OS_CYGWIN
    /*
     * Set COM mask. Register event for receive buffer getting data.
     */
    if (!SetCommMask(handle, EV_RXCHAR))
    {
        std::cout << "Error setting COM mask: " << GetLastError() << '\n';
        return false;
    }

    /*
     * Set com port paramaters.
     */
    DCB dcb = {0};
    dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(handle, &dcb))
    {
        std::cout << "Error getting comm state: " << GetLastError() << '\n';
        return false;
    }

    dcb.BaudRate = 9600;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;

    if (!SetCommState(handle, &dcb))
    {
        std::cout << "Error setting comm state: " << GetLastError() << '\n';
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
        std::cout << "Error settings timeouts: " << GetLastError() << '\n';
        return false;
    }
#endif

    port_configured = true;
    return true;
}

bool ComPort::start_reading()
{
    if (is_reading())
    {
        std::cout << "Already reading from port.\n";
        return false;
    }
    if (!port_open)
    {
        std::cout << "Cannot start reading from port before opening.\n";
        return false;
    }
    if (!port_configured)
    {
        std::cout << "Must configure port before starting to read from it.\n";
        return false;
    }

    this->port_reading.store(true);

    this->buffer->clear();

#ifdef OS_CYGWIN
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

    thread_handle = std::thread(&ComPort::async_receive, (void*)this);
    thread_handle.detach();

    DWORD wait_rv = WaitForSingleObject(thread_started, INFINITE);

    assert(wait_rv == WAIT_OBJECT_0);

    CloseHandle(thread_started);
    handle = INVALID_HANDLE_VALUE;
#endif

    return true;
}

void ComPort::stop_reading()
{
#ifdef OS_CYGWIN
    this->port_reading.store(false);
    SetEvent(thread_term);
#endif
}

std::vector<std::string> ComPort::find_ports()
{
    available_ports.clear();

#ifdef OS_CYGWIN
    for (std::size_t i = COM_BEG; i < COM_END; i++)
    {
        std::string com_port_str = COM_PORT_PREFIX + std::to_string(i);

        std::cout << "Checking COM" << i << "...\n";
        handle = CreateFile(com_port_str.c_str(),  // filename
                            GENERIC_READ,          // access method
                            0,                     // cannot share
                            NULL,                  // no security attributes
                            OPEN_EXISTING,         // file action, value for serial ports
                            0,                     // FILE_FLAG_OVERLAPPED TODO change
                            NULL);                 // ignored

        if (handle != INVALID_HANDLE_VALUE)
        {
            std::cout << "COM" << i << " available\n";
            available_ports.push_back(i);
        }
        CloseHandle(handle);
    }
#endif
    return available_ports;
}

bool ComPort::open(const std::string& port)
{
    if (port_open)
    {
        std::cout << "Port is already opened\n";
        return false;
    }
    std::string com_port_str = COM_PORT_PREFIX + port;

#ifdef OS_CYGWIN
    handle = CreateFile(com_port_str.c_str(),  // filename
                        GENERIC_READ,          // access method
                        0,                     // cannot share
                        NULL,                  // no security attributes
                        OPEN_EXISTING,         // file action, value for serial ports
                        0,                     // FILE_FLAG_OVERLAPPED TODO change
                        NULL);                 // ignored

    if (handle != INVALID_HANDLE_VALUE)
    {
        std::cout << "Successfully opened " << com_port_str << '\n';
        port_open = true;
        port_name = com_port_str;
        return true;
    }
#endif
    return false;
}

bool ComPort::auto_open()
{
    for (std::size_t i = COM_BEG; i < COM_END; i++)
    {
        std::string com_port_str = COM_PORT_PREFIX + std::to_string(i);

        std::cout << "Attempting to open COM" << i << "...\n";
#ifdef OS_CYGWIN
        handle = CreateFile(com_port_str.c_str(),  // filename
                            GENERIC_READ,          // access method
                            0,                     // cannot share
                            NULL,                  // no security attributes
                            OPEN_EXISTING,         // file action, value for serial ports
                            0,                     // FILE_FLAG_OVERLAPPED TODO change
                            NULL);                 // ignored

        if (handle != INVALID_HANDLE_VALUE)
        {
            std::cout << "Successfully pened COM" << i << '\n';
            port_open = true;
            open_port = i;
            return true;
        }
#endif
    }
    std::cout << "Could not find an available COM port. Aborting.\n";
    return false;
}

void ComPort::close()
{
}

unsigned ComPort::async_receive(void* params)
{
    ComPort* com_ptr = static_cast<ComPort*>(params);
    bool keep_processing = true;
#ifdef OS_CYGWIN
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
            std::cout << "GetLastError() = " << GetLastError() << '\n';
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
                    std::cout << "Error: Serial reading thread failed!";
                    assert(0);
                }
                ResetEvent(ov.hEvent);
                break;
            }
        }
    }
#endif
    return 0;
}
