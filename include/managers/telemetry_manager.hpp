#ifndef TELEMETRY_MANAGER_HPP
#define TELEMETRY_MANAGER_HPP

#include <cmath>
#include <memory>
#include <vector>

#include "resource_manager.hpp"
#include "serial_port.hpp"
#include "shared.hpp"

struct TelemetryData;
struct TelemetryFormat;

struct TelemetryFormat
{
    TelemetryFormat(std::size_t packet_len_,
                    char start_symbol_,
                    char stop_symbol_,
                    std::size_t conversion_factor_,
                    std::size_t element_size_,
                    std::vector<std::size_t> accel_offsets_,
                    std::vector<std::size_t> rot_rate_offsets_) :
        packet_len(packet_len_),
        start_symbol(start_symbol_),
        stop_symbol(stop_symbol_),
        conversion_factor(conversion_factor_),
        element_size(element_size_),
        accel_offsets(accel_offsets_),
        rot_rate_offsets(rot_rate_offsets_)
    {}

    const std::size_t packet_len;
    const char start_symbol;
    const char stop_symbol;
    const std::size_t conversion_factor;
    const std::size_t element_size;
    const std::vector<std::size_t> accel_offsets{};
    const std::vector<std::size_t> rot_rate_offsets{};
};

/*
 * Telemetry data which comes from the telemetry board via an external serial
 * connection.
 */
class TelemetryData
{
public:
    explicit TelemetryData(TelemetryFormat fmt_) : fmt(fmt_) {}

    DroneData& operator=(const TelemetryData&);

    bool extract_packet_data(const std::string& packet)
    {
        if (packet.size() != fmt.packet_len)
        {
            std::cout << "Packet incorrect length. Aborting.\n";
            return false;
        }

        accel.x = std::stof(packet.substr(fmt.accel_offsets[0], fmt.element_size)) / fmt.conversion_factor;
        accel.y = std::stof(packet.substr(fmt.accel_offsets[1], fmt.element_size)) / fmt.conversion_factor;
        accel.z = std::stof(packet.substr(fmt.accel_offsets[2], fmt.element_size)) / fmt.conversion_factor;

        rot_rate.x = std::stof(packet.substr(fmt.rot_rate_offsets[0], fmt.element_size)) / fmt.conversion_factor;
        rot_rate.y = std::stof(packet.substr(fmt.rot_rate_offsets[1], fmt.element_size)) / fmt.conversion_factor;
        rot_rate.z = std::stof(packet.substr(fmt.rot_rate_offsets[2], fmt.element_size)) / fmt.conversion_factor;

        return true;
    }

    // TODO change this to convert accel->pos and rot_rate->rot, not just pass
    // the values through.
    DroneData get_raw_drone_data() const
    {
        return DroneData(accel, rot_rate);
    }

    glm::vec3 get_accel() const { return accel; }
    glm::vec3 get_rot_rate() const { return rot_rate; }
private:
    TelemetryFormat fmt;

    glm::vec3 accel{};
    glm::vec3 rot_rate{};
};

/*
 * Definitions requiring complete definitions of DroneData and TelemetryData.
 */
DroneData& DroneData::operator=(const TelemetryData& tel)
{
    position = tel.get_accel();
    orientation = tel.get_rot_rate();

    return *this;
}

class TelemetryManager
{
public:
    TelemetryManager(std::size_t packet_len_,
                     char start_symbol_,
                     char stop_symbol_,
                     std::size_t float_conversion_factor_,
                     std::size_t float_format_len_,
                     std::vector<std::size_t> accel_offsets_,
                     std::vector<std::size_t> rot_rate_offsets_,
                     SerialPort* serial_port_,
                     DroneData* drone_data_,
                     ResourceManager* resource_manager_,
                     std::shared_ptr<BoundedBuffer<char>> telemetry_buffer_) :
        packet_len(packet_len_),
        start_symbol(start_symbol_),
        stop_symbol(stop_symbol_),
        float_conversion_factor(float_conversion_factor_),
        float_format_len(float_format_len_),
        accel_offsets(accel_offsets_),
        rot_rate_offsets(rot_rate_offsets_),
        serial_port(serial_port_),
        drone_data(drone_data_),
        resource_manager(resource_manager_),
        telemetry_buffer(telemetry_buffer_)
    {}

    bool init();

    DroneData filter_data();
    std::shared_ptr<std::string> build_latest_packet();
    bool process_telemetry();
private:
    const std::size_t packet_len;
    const char start_symbol;
    const char stop_symbol;
    const std::size_t float_conversion_factor;
    const std::size_t float_format_len;
    const std::vector<std::size_t> accel_offsets;
    const std::vector<std::size_t> rot_rate_offsets;

    std::unique_ptr<TelemetryFormat> fmt;
    SerialPort* serial_port;
    DroneData* drone_data;
    ResourceManager* resource_manager;

    std::shared_ptr<BoundedBuffer<char>> telemetry_buffer;
    bool build_new_packet = true;
    std::string latest_packet{};

    static constexpr std::size_t RAW_DATA_BUF_MAXLEN = 32;
    std::vector<DroneData> raw_data_buf;
    // float integration_sum = 0.0f;  // TODO
};

bool TelemetryManager::init()
{
    /*
     * Initialize communications interfaces.
     */
    fmt = std::make_unique<TelemetryFormat>(
        packet_len,
        start_symbol,
        stop_symbol,
        float_conversion_factor,
        float_format_len,
        accel_offsets,
        rot_rate_offsets
    );

    return true;
}

/*
 * Implement simple moving average filter.
 */
DroneData TelemetryManager::filter_data()
{
    DroneData sum{};

    for (auto& e : raw_data_buf)
        sum += e;

    return DroneData{sum.position / (float)raw_data_buf.size(),
                     sum.orientation / (float)raw_data_buf.size()};
}

/*
 * The telemetry-receiving module of the application processes drone data in a
 * streaming fashion. That is, only the most recent data from the drone is used.
 * The graphics portion of the application runs at 60Hz. At the moment the
 * telemetry processing module also runs at this same frequency of 60Hz, since
 * it is part of the main application loop. The main loop of the drone will
 * likely run at a higher rate, say 100-200Hz. Therefore, many packets will
 * be dropped. At the moment that is fine, but it means that the viewer won't be
 * able to run any kind of controls filters on the data since it does not
 * receive all of it.
 *
 * Now, onto this function. Since we are streaming the data in real-time and
 * don't care about loss, only the minimal amount of raw data is kept. To be
 * precise, the raw telemetry buffer is the size of two full packets, minus one.
 * This makes packet construction both simple and quick. As a quick example, say
 * the packet has a start symbol A, a stop symbol C, and a length of 3. Then the
 * following permutation are possible when the buffer is full:
 *
 *         1. | B | A | C | B | A |
 *   back  2. | C | B | A | C | B |  front
 *         3. | A | C | B | A | C |
 *
 * By searching first for the start symbol, then the stop symbol while ensuring
 * correct length, a complete uncorrupted packet can be built consistently.
 */
std::shared_ptr<std::string> TelemetryManager::build_latest_packet()
{
    char tmp{};
    std::shared_ptr<char> result{};

    // If necessary, start building new packet. First, find the start symbol.
    result = telemetry_buffer->try_pop();
    if (build_new_packet)
    {
        while (result)
        {
            if (*result == fmt->start_symbol)
                break;
            result = telemetry_buffer->try_pop();
        }
    }

    // Extract the rest of the packet.
    while (result)
    {
        if (*result == fmt->stop_symbol)
            break;
        latest_packet += *result;
        result = telemetry_buffer->try_pop();
    }

    // Complete packet not built yet.
    if (latest_packet.size() < fmt->packet_len)
    {
        build_new_packet = false;
        return nullptr;
    }

    // Finish building packet.
    if ((latest_packet.size() == fmt->packet_len) &&
        (latest_packet[0] == fmt->start_symbol))
    {
        std::string rv = latest_packet;
        latest_packet.clear();
        build_new_packet = true;
        return std::make_shared<std::string>(rv);
    }

    // Packet is complete but corrupted, restart.
    latest_packet.clear();
    build_new_packet = true;
    return nullptr;
}

bool TelemetryManager::process_telemetry()
{
    auto packet_str = std::make_shared<std::string>();
    if (serial_port->is_reading())
    {
        packet_str = build_latest_packet();
        if (packet_str)
        {
            std::cout << "packet_str: " << *packet_str << '\n';
        }
    }
    else
    {
        return true;
    }

    auto telemetry_data = std::make_shared<TelemetryData>(*fmt);
    if (packet_str)
    {
        if (!telemetry_data->extract_packet_data(*packet_str)) return false;
        {
            if (raw_data_buf.size() >= RAW_DATA_BUF_MAXLEN)
                raw_data_buf.erase(raw_data_buf.begin());
            raw_data_buf.push_back(telemetry_data->get_raw_drone_data());
        }
    }

    // Filter input.
    {
        std::lock_guard<std::mutex> g(resource_manager->drone_data_mutex);
        *drone_data = filter_data();
    }

    // // Integrate. TODO just store integrated value in z value for now.
    // {
    //     if (!std::isnan(drone_data->position.x))
    //         integration_sum += drone_data->position.x * 0.01;
    //     std::lock_guard<std::mutex> g(resource_manager->drone_data_mutex);
    //     drone_data->position.z = integration_sum;
    //
    //     std::cout << "drone.x = " << drone_data->position.x << '\n';
    //     std::cout << "integration_sum = " << integration_sum << '\n';
    //     std::cout << "drone.z = " << drone_data->position.z << '\n';
    // }

    return true;
}

#endif /* TELEMETRY_MANAGER_HPP */
