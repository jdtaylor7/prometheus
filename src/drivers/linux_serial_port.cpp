#include "linux_serial_port.hpp"

#ifdef OS_LINUX

LinuxSerialPort::LinuxSerialPort(
        std::shared_ptr<BoundedBuffer<char>> buffer_,
        LinuxSerialPortConfig const* cfg_) :
    buffer{buffer_},
    cfg(cfg_),
    stream{}
{
}

LinuxSerialPort::~LinuxSerialPort()
{
}

/*
 * All relevant external serial devices are listed in /dev/serial. These entries
 * then symlink to the actual enpoint in /dev (/dev/ttyS0, /dev/ttyACM0, etc.).
 * Additionally, /dev/serial does not exist if no devices are plugged into the
 * system. Again, this method finds physical USB->UART devices at the very
 * least, not virtual ports or potentially other serial devices. For our
 * purposes that's sufficient.
 */
std::vector<std::string> LinuxSerialPort::find_ports()
{
    namespace fs = std::filesystem;

    available_ports.clear();
    fs::path serial_device_path{"/dev/serial/by-id/"};
    fs::path dev_dir{"/dev/"};

    if (serial_device_path.empty())
    {
        std::cerr << "Serial device directory does not exist.\n";
        return std::vector<std::string>{};
    }

    for (auto& entry : fs::directory_iterator(serial_device_path))
    {
        fs::path dev_file = fs::read_symlink(entry).filename();
        fs::path dev_path = dev_dir / dev_file;
        std::cout << dev_path.string() << '\n';
        available_ports.push_back(dev_path.string());
    }

    return available_ports;
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
        std::cout << "opening port " << port << '\n';
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

bool LinuxSerialPort::config()
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

    std::cout << "configuring port\n";
    if (cfg->br == LibSerial::BaudRate::BAUD_9600)
        std::cout << "baud rate is correct\n";
    else
        std::cout << "baud rate is wrong!\n";

    stream.SetBaudRate(cfg->br);
    stream.SetCharacterSize(cfg->cs);
    stream.SetFlowControl(cfg->fc);
    stream.SetParity(cfg->py);
    stream.SetStopBits(cfg->sb);

    port_configured = true;
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
