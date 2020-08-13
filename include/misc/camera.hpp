#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <memory>
#include <mutex>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "resource_manager.hpp"
#include "shared.hpp"

enum class CameraSpeedSetting
{
    Normal,
    Fast,
};

class Camera
{
public:
    Camera(ResourceManager* rm_,
           std::size_t screen_width,
           std::size_t screen_height,
           glm::vec3 room_dimensions_,
           glm::vec3 position_,
           glm::vec3 target_) :
        rm(rm_),
        lastx(screen_width / 2),
        lasty(screen_height / 2),
        room_dimensions(room_dimensions_),
        position(position_),
        target(target_),
        front(target_ - position_)
    {
    }

    friend std::ostream& operator<<(std::ostream&, const Camera&);

    glm::vec3 get_position() const;
    glm::vec3 get_target() const;
    glm::vec3 get_front() const;
    glm::vec3 get_up() const;
    float get_fov() const;
    float get_yaw() const { return yaw; };
    float get_pitch() const { return pitch; };

    void set_speed_modifier(CameraSpeedSetting);
    void set_position(glm::vec3);
    void set_target_and_front(glm::vec3);

    void update_position(GLFWwindow* window);
    void update_angle(double xpos, double ypos);
    void update_pov(double yoffset);
    void process_frame();
private:
    /*
     * Data managers.
     */
    ResourceManager* rm;

    /*
     * Constants.
     */
    const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    const float mouse_sensitivity = 0.05f;
    const float collision_bias = 0.2f;

    /*
     * Internal state.
     */
    float delta_time = 0.0f;
    float last_frame = 0.0f;

    static constexpr float CAMERA_SPEED_NORMAL = 2.5f;
    static constexpr float CAMERA_SPEED_FAST = 5.0f;
    float camera_speed_modifier = CAMERA_SPEED_NORMAL;

    bool first_mouse = true;

    glm::vec3 room_dimensions;

    float lastx;
    float lasty;

    /*
     * Output state.
     */
    float fov = 45.0f;

    float yaw = -90.0f;
    float pitch = 0.0f;

    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 front;

    inline void constrain_to_boundary();
};

std::ostream& operator<<(std::ostream& os, const Camera& cam)
{
    os << "camera.position.x = " << cam.position.x << '\n';
    os << "camera.position.y = " << cam.position.y << '\n';
    os << "camera.position.z = " << cam.position.z << '\n';

    os << "camera.target.x = " << cam.target.x << '\n';
    os << "camera.target.y = " << cam.target.y << '\n';
    os << "camera.target.z = " << cam.target.z << '\n';

    return os;
}

glm::vec3 Camera::get_position() const
{
    return position;
}

glm::vec3 Camera::get_target() const
{
    return target;
}

glm::vec3 Camera::get_front() const
{
    return front;
}

glm::vec3 Camera::get_up() const
{
    return up;
}

float Camera::get_fov() const
{
    return fov;
}

void Camera::set_speed_modifier(CameraSpeedSetting setting)
{
    switch (setting)
    {
        case (CameraSpeedSetting::Normal):
        {
            camera_speed_modifier = CAMERA_SPEED_NORMAL;
            break;
        }
        case (CameraSpeedSetting::Fast):
        {
            camera_speed_modifier = CAMERA_SPEED_FAST;
            break;
        }
    }
}

void Camera::set_position(glm::vec3 pos)
{
    position = pos;
}

void Camera::set_target_and_front(glm::vec3 tar)
{
    target = tar;
    front = target - position;
}

/*
 * Prevent camera from leaving enclosure.
 */
inline void Camera::constrain_to_boundary()
{
    if (position.x > room_dimensions.x / 2 - collision_bias)
        position.x = room_dimensions.x / 2 - collision_bias;
    else if (position.x < (-1 * room_dimensions.x / 2) + collision_bias)
        position.x = (-1 * room_dimensions.x / 2 + collision_bias);

    // TODO fix
    // if (position.y > room_dimensions.y - collision_bias)
    //     position.y = room_dimensions.y - collision_bias;
    // else if (position.y < 0 + collision_bias)
    //     position.y = 0 + collision_bias;
    if (position.y > room_dimensions.y - 2 - collision_bias)
        position.y = room_dimensions.y - 2 - collision_bias;
    else if (position.y < -2 + collision_bias)
        position.y = -2 + collision_bias;

    if (position.z > room_dimensions.z / 2 - collision_bias)
        position.z = room_dimensions.z / 2 - collision_bias;
    else if (position.z < (-1 * room_dimensions.z / 2) + collision_bias)
        position.z = (-1 * room_dimensions.z / 2 + collision_bias);
}

void Camera::update_position(GLFWwindow* window)
{
    float camera_speed = camera_speed_modifier * delta_time;

    std::lock_guard<std::mutex> g(rm->camera_data_mutex);

    // Camera WASD.
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        position += camera_speed * front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        position -= camera_speed * front;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        position -= glm::normalize(glm::cross(front, up)) * camera_speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        position += glm::normalize(glm::cross(front, up)) * camera_speed;

    // Camera up/down.
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        position += camera_speed * up;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        position -= camera_speed * up;

    // Camera speed.
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        set_speed_modifier(CameraSpeedSetting::Fast);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
        set_speed_modifier(CameraSpeedSetting::Normal);

    constrain_to_boundary();
}

void Camera::update_angle(double xpos, double ypos)
{
    std::lock_guard<std::mutex> g(rm->camera_data_mutex);

    if (first_mouse)
    {
        lastx = xpos;
        lasty = ypos;
        first_mouse = false;
    }

    float xoffset = xpos - lastx;
    float yoffset = lasty - ypos;
    lastx = xpos;
    lasty = ypos;

    xoffset *= mouse_sensitivity;
    yoffset *= mouse_sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(direction);
    target = position - front;
}

void Camera::update_pov(double yoffset)
{
    std::lock_guard<std::mutex> g(rm->camera_data_mutex);

    fov -= yoffset;

    if (fov <= 1.0f)
        fov = 1.0f;
    else if (fov >= 45.0f)
        fov = 45.0f;
}

/*
 * Maintain consistent movement speed regardless of processing power.
 */
void Camera::process_frame()
{
    std::lock_guard<std::mutex> g(rm->camera_data_mutex);

    float current_frame = glfwGetTime();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;
}

#endif /* CAMERA_HPP */
