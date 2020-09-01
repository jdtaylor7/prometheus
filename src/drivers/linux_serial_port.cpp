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
    close();
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

    if (!fs::exists(serial_device_path))
    {
        logger.log(LogLevel::info, "Serial device directory does not exist\n");
        return std::vector<std::string>{};
    }

    for (auto& entry : fs::directory_iterator(serial_device_path))
    {
        fs::path dev_file = fs::read_symlink(entry).filename();
        fs::path dev_path = dev_dir / dev_file;
        available_ports.push_back(dev_path.string());
    }

    return available_ports;
}

bool LinuxSerialPort::open(const std::string& port)
{
    if (port_open)
    {
        logger.log(LogLevel::warning, "Port is already opened\n");
        return false;
    }

    try
    {
        logger.log(LogLevel::info, "Opening ", port, '\n');
        stream.Open(port.c_str());
    }
    catch (const LibSerial::OpenFailed&)
    {
        logger.log(LogLevel::warning, "Failed to open serial port: ", port, '\n');
        return false;
    }
    port_open = true;
    port_name = port;
    return true;
}

bool LinuxSerialPort::auto_open()
{
    find_ports();
    if (available_ports.size() == 1)
    {
        logger.log(LogLevel::info, "Automatically opening ", available_ports[0], '\n');
        open(available_ports[0]);
        config();
        return true;
    }
    else if (available_ports.size() > 1)
    {
        logger.log(LogLevel::info, "Not auto-opening any ports: More than 1 available\n");
        return false;
    }
    else
    {
        logger.log(LogLevel::info, "Not auto-opening any ports: None available\n");
        return false;
    }
}

void LinuxSerialPort::close()
{
    stop_reading();
    port_open = false;
    port_configured = false;
    port_name = "";

    buffer->clear();
    available_ports.clear();

    stream.Close();
}

bool LinuxSerialPort::config()
{
    if (!port_open)
    {
        logger.log(LogLevel::warning, "Cannot configure port before opening\n");
        return false;
    }
    if (port_configured)
    {
        logger.log(LogLevel::warning, "Port has already been configured\n");
        return false;
    }

    logger.log(LogLevel::info, "Configuring ", port_name, "\n");

    if (cfg)
    {
        stream.SetBaudRate(cfg->br);
        stream.SetCharacterSize(cfg->cs);
        stream.SetFlowControl(cfg->fc);
        stream.SetParity(cfg->py);
        stream.SetStopBits(cfg->sb);
    }
    else
    {
        logger.log(LogLevel::error, "LinuxSerialPort::config: cfg pointer is \
            null");
        return false;
    }

    port_configured = true;
    return true;
}

bool LinuxSerialPort::start_reading()
{
    if (is_reading())
    {
        logger.log(LogLevel::warning, "Already reading from port\n");
        return false;
    }
    if (!port_open)
    {
        logger.log(LogLevel::warning, "Cannot read from port before opening it\n");
        return false;
    }
    if (!port_configured)
    {
        logger.log(LogLevel::warning, "Must configure port before reading from it\n");
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
