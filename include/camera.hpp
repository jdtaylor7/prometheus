#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <memory>
#include <mutex>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "resource_manager.hpp"
#include "shared.hpp"

class Camera
{
public:
    Camera(ResourceManager* rm_,
           std::size_t screen_width,
           std::size_t screen_height,
           float horizontal_boundary_,
           float top_boundary_,
           CameraData* camera_data_) :
        rm(rm_),
        lastx(screen_width / 2),
        lasty(screen_height / 2),
        horizontal_boundary(horizontal_boundary_ - 0.2f),
        top_boundary(top_boundary_ - 0.2f),
        camera_data(camera_data_),
        front(camera_data_->target - camera_data_->position)
        {}

    glm::vec3 get_pos() const;
    glm::vec3 get_front() const;
    glm::vec3 get_up() const;
    float get_fov() const;

    // void update_pos(GLFWwindow* window);
    // void update_angle(double xpos, double ypos);
    // void update_pov(double yoffset);
    // void update_frames();
private:
    ResourceManager* rm;

    CameraData* camera_data;
    glm::vec3 front;
    const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    float delta_time = 0.0f;
    float last_frame = 0.0f;

    float lastx;
    float lasty;

    const float mouse_sensitivity = 0.05f;

    float yaw = -90.0f;
    float pitch = 0.0f;

    bool first_mouse = true;

    float fov = 45.0f;

    float horizontal_boundary;
    float top_boundary;
    const float bottom_boundary = 0.2f;

    // inline void constrain_to_boundary();
};

glm::vec3 Camera::get_pos() const
{
    return camera_data->position;
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

// /*
//  * Prevent camera from leaving enclosure.
//  */
// inline void Camera::constrain_to_boundary()
// {
//     if (pos.x > horizontal_boundary)
//         pos.x = horizontal_boundary;
//     else if (pos.x < (-1 * horizontal_boundary))
//         pos.x = (-1 * horizontal_boundary);
//
//     if (pos.y > top_boundary)
//         pos.y = top_boundary;
//     else if (pos.y < bottom_boundary)
//         pos.y = bottom_boundary;
//
//     if (pos.z > horizontal_boundary)
//         pos.z = horizontal_boundary;
//     else if (pos.z < (-1 * horizontal_boundary))
//         pos.z = (-1 * horizontal_boundary);
// }
//
// void Camera::update_pos(GLFWwindow* window)
// {
//     float camera_speed = 2.5f * delta_time;
//
//     std::lock_guard<std::mutex> g(rm->camera_data_mutex);
//
//     if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
//     {
//         pos += camera_speed * front;
//         constrain_to_boundary();
//     }
//     if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
//     {
//         pos -= camera_speed * front;
//         constrain_to_boundary();
//     }
//     if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
//     {
//         pos -= glm::normalize(glm::cross(front, up)) * camera_speed;
//         constrain_to_boundary();
//     }
//     if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
//     {
//         pos += glm::normalize(glm::cross(front, up)) * camera_speed;
//         constrain_to_boundary();
//     }
// }
//
// void Camera::update_angle(double xpos, double ypos)
// {
//     std::lock_guard<std::mutex> g(rm->camera_data_mutex);
//
//     if (first_mouse)
//     {
//         lastx = xpos;
//         lasty = ypos;
//         first_mouse = false;
//     }
//
//     float xoffset = xpos - lastx;
//     float yoffset = lasty - ypos;
//     lastx = xpos;
//     lasty = ypos;
//
//     xoffset *= mouse_sensitivity;
//     yoffset *= mouse_sensitivity;
//
//     yaw += xoffset;
//     pitch += yoffset;
//
//     if (pitch > 89.0f)
//         pitch = 89.0f;
//     if (pitch < -89.0f)
//         pitch = -89.0f;
//
//     glm::vec3 direction;
//     direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
//     direction.y = sin(glm::radians(pitch));
//     direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
//     front = glm::normalize(direction);
// }
//
// void Camera::update_pov(double yoffset)
// {
//     std::lock_guard<std::mutex> g(rm->camera_data_mutex);
//
//     fov -= yoffset;
//
//     if (fov <= 1.0f)
//         fov = 1.0f;
//     else if (fov >= 45.0f)
//         fov = 45.0f;
// }
//
// /*
//  * Maintain consistent movement speed regardless of processing power.
//  */
// void Camera::update_frames()
// {
//     std::lock_guard<std::mutex> g(rm->camera_data_mutex);
//
//     float current_frame = glfwGetTime();
//     delta_time = current_frame - last_frame;
//     last_frame = current_frame;
// }

#endif /* CAMERA_HPP */
