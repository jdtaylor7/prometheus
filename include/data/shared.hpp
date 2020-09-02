#ifndef SHARED_HPP
#define SHARED_HPP

#include <vector>

struct DroneData;
struct TelemetryData;

/*
 * Data representing the state of the drone in the viewer application window.
 */
struct DroneData
{
    DroneData() : position{}, orientation{} {}
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

    void operator+=(const DroneData& rhs)
    {
        position += rhs.position;
        orientation += rhs.orientation;
    }

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

/*
 * Global constants.
 */
const DroneData INITIAL_DRONE_DATA = DroneData(glm::vec3(0.0, 0.3, 0.0),
                                               glm::vec3(0.0, 0.0, 0.0));
constexpr float DRONE_OFFSET_TOP = 1.5;
constexpr float DRONE_OFFSET_BOT = 0.5;

const glm::vec3 CAMERA_POSITION_HEADON = glm::vec3(0.0, 2.0, 11.5);
const glm::vec3 CAMERA_FRONT_HEADON = glm::vec3(0.0, 0.0, -1.0);
const float CAMERA_PITCH_HEADON = 0.0;
const float CAMERA_YAW_HEADON = -90.0;

const glm::vec3 CAMERA_POSITION_OVERHEAD = glm::vec3(6.952, 5.641, 9.737);
const glm::vec3 CAMERA_FRONT_OVERHEAD = glm::vec3(-0.579, -0.342, -0.740);
const float CAMERA_PITCH_OVERHEAD = -20.0;
const float CAMERA_YAW_OVERHEAD = -128.0;

const glm::vec3 CAMERA_POSITION_CLOSE = glm::vec3(0.633, 3.184, 4.162);
const glm::vec3 CAMERA_FRONT_CLOSE = glm::vec3(-0.143, -0.096, -0.985);
const float CAMERA_PITCH_CLOSE = -5.5;
const float CAMERA_YAW_CLOSE = -98.25;

#endif /* SHARED_HPP */
