#ifndef SHARED_HPP
#define SHARED_HPP

struct DroneData
{
    DroneData(glm::vec3 p, glm::vec3 o) : position(p), orientation(o) {}

    glm::vec3 position;
    glm::vec3 orientation;

    const float size = 0.2f;
};

#endif /* SHARED_HPP */
