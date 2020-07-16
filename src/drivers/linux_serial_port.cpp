#include "linux_serial_port.hpp"

#ifdef OS_LINUX

LinuxSerialPort::LinuxSerialPort(
        std::shared_ptr<BoundedBuffer<char>> buffer_) :
    stream{},
    buffer{buffer_}
{
}

LinuxSerialPort::~LinuxSerialPort()
{
}

std::vector<std::string> LinuxSerialPort::find_ports() const
{
    return std::vector<std::string>{};
}

bool LinuxSerialPort::open(const std::string& port)
{
    if (port_open)
    {
        std::cout << "Port is already opened\n";
        return false;
    }

    try
    {
        // TODO reinstate when necessary
        // stream.Open(port.c_str());
        stream.Open("/dev/ttyACM0");
    }
    catch (const LibSerial::OpenFailed&)
    {
        std::cerr << "Failed to open serial port: " << port << '\n';
        return false;
    }
    port_open = true;
    port_name = port;
    return true;
}

bool LinuxSerialPort::config(const LinuxSerialPortConfig& cfg)
{
    if (!port_open)
    {
        std::cout << "Canont configure port before opening.\n";
        return false;
    }
    if (port_configured)
    {
        std::cout << "Port has already been configured.\n";
        return false;
    }

    stream.SetBaudRate(cfg.br);
    stream.SetCharacterSize(cfg.cs);
    stream.SetFlowControl(cfg.fc);
    stream.SetParity(cfg.py);
    stream.SetStopBits(cfg.sb);

    return true;
}

bool LinuxSerialPort::start_reading()
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

    port_reading.store(true);

    std::thread t1([&](){
        while (port_reading.load())
        {
            if (stream.IsDataAvailable())
            {
                using namespace std::chrono_literals;

                char recv_byte;
                stream.get(recv_byte);
                buffer->force_push(recv_byte);

                std::this_thread::sleep_for(1us);
            }
        }
    });
    t1.detach();

    return true;
}

void LinuxSerialPort::stop_reading()
{
    port_reading.store(false);
}

#endif /* OS_LINUX */
