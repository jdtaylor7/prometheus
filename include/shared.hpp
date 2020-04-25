#ifndef SHARED_HPP
#define SHARED_HPP

struct DroneData
{
    DroneData(glm::vec3 p, glm::vec3 o) : position(p), orientation(o) {}
    // DroneData(glm::vec3 p, glm::vec3 o) : position(p), orientation(o) {}
    glm::vec3 position;
    glm::vec3 orientation;
};

struct CameraData
{
    CameraData(glm::vec3 p, glm::vec3 t) : position(p), target(t) {}
    glm::vec3 position;
    glm::vec3 target;
};

#endif /* SHARED_HPP */
