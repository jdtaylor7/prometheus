#ifndef SHARED_HPP
#define SHARED_HPP

#include <iostream>
#include <vector>

struct DroneData;
struct TelemetryData;
struct TelemetryFormat;

/*
 * Data representing the state of the drone in the viewer application window.
 */
struct DroneData
{
    DroneData(glm::vec3 p, glm::vec3 o) : position(p), orientation(o) {}

    DroneData(const DroneData& rhs) :
        position(rhs.position),
        orientation(rhs.orientation) {}

    DroneData& operator=(const DroneData& rhs)
    {
        if (&rhs != this)
        {
            position = rhs.position;
            orientation = rhs.orientation;
        }

        return *this;
    }

    DroneData& operator=(const TelemetryData&);

    friend std::ostream& operator<<(std::ostream&, const DroneData&);

    glm::vec3 position{};
    glm::vec3 orientation{};

    const float size = 0.2f;
};

std::ostream& operator<<(std::ostream& os, const DroneData& data)
{
    os << "drone_data.position.x = " << data.position.x << '\n';
    os << "drone_data.position.y = " << data.position.y << '\n';
    os << "drone_data.position.z = " << data.position.z << '\n';

    os << "drone_data.orientation.x = " << data.orientation.x << '\n';
    os << "drone_data.orientation.y = " << data.orientation.y << '\n';
    os << "drone_data.orientation.z = " << data.orientation.z << '\n';

    return os;
}

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
    DroneData get_drone_data() const
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

/*
 * Global constants.
 */
const DroneData INITIAL_DRONE_DATA = DroneData(glm::vec3(0.0, 0.1, 0.0),
                                               glm::vec3(0.0, 0.0, 0.0));
const glm::vec3 INITIAL_CAMERA_POSITION = glm::vec3(0.0, 1.0, 4.0);
const glm::vec3 INITIAL_CAMERA_TARGET = glm::vec3(0.0, 1.0, 3.0);

#endif /* SHARED_HPP */
