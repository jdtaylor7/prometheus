#ifndef SHARED_HPP
#define SHARED_HPP

#include <iostream>

struct DroneData
{
    DroneData(glm::vec3 p, glm::vec3 o) : position(p), orientation(o) {}

    DroneData(const DroneData& rhs) :
        position(rhs.position),
        orientation(rhs.orientation) {}

    DroneData& operator=(const DroneData&);

    friend std::ostream& operator<<(std::ostream&, const DroneData&);

    glm::vec3 position;
    glm::vec3 orientation;

    const float size = 0.2f;
};

DroneData& DroneData::operator=(const DroneData& rhs)
{
    if (&rhs != this)
    {
        position = rhs.position;
        orientation = rhs.orientation;
    }

    return *this;
}

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

const DroneData INITIAL_DRONE_DATA = DroneData(glm::vec3(0.0, 0.1, 0.0),
                                               glm::vec3(0.0, 0.0, 0.0));
const glm::vec3 INITIAL_CAMERA_POSITION = glm::vec3(0.0, 1.0, 4.0);
const glm::vec3 INITIAL_CAMERA_TARGET = glm::vec3(0.0, 1.0, 3.0);

#endif /* SHARED_HPP */
