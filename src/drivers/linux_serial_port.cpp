#include "linux_serial_port.hpp"

#ifdef OS_LINUX

LinuxSerialPort::LinuxSerialPort(
        std::shared_ptr<BoundedBuffer<char>> byte_buffer_) :
    stream{},
    byte_buffer{byte_buffer_}
{
}

LinuxSerialPort::~LinuxSerialPort()
{
}

bool LinuxSerialPort::open(const std::string& port)
{
    try
    {
        stream.Open(port.c_str());
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

void LinuxSerialPort::config(const LinuxSerialPortConfig& cfg)
{
    stream.SetBaudRate(cfg.br);
    stream.SetCharacterSize(cfg.cs);
    stream.SetFlowControl(cfg.fc);
    stream.SetParity(cfg.py);
    stream.SetStopBits(cfg.sb);
}

std::vector<std::string> LinuxSerialPort::find_ports() const
{
    return std::vector<std::string>{};
}

bool LinuxSerialPort::is_open() const
{
    return port_open;
}

bool LinuxSerialPort::is_reading() const
{
    return port_reading.load();
}

std::string LinuxSerialPort::get_port_name() const
{
    return std::string{};
}

void LinuxSerialPort::start_reading()
{
    port_reading.store(true);

    std::thread t1([&](){
        while (port_reading.load())
        {
            if (stream.IsDataAvailable())
            {
                using namespace std::chrono_literals;

                char recv_byte;
                stream.get(recv_byte);
                byte_buffer->force_push(recv_byte);

                std::this_thread::sleep_for(1us);
            }
        }
    });
    t1.detach();
}

void LinuxSerialPort::stop_reading()
{
    port_reading.store(false);
}

#endif /* OS_LINUX */
