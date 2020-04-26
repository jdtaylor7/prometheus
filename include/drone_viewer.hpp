#ifndef DRONE_VIEWER_HPP
#define DRONE_VIEWER_HPP

#include <iostream>
#include <memory>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "camera.hpp"
#include "glfw_manager.hpp"
#include "imgui_manager.hpp"
#include "opengl_manager.hpp"
#include "resource_manager.hpp"
#include "shader.hpp"
#include "shared.hpp"
#include "vertex_data.hpp"
#include "viewer_mode.hpp"

class DroneViewer
{
public:
    bool init();
    bool is_running() const;
    void process_frame();
private:
    /*
     * Constants.
     */
    static constexpr std::size_t SCREEN_WIDTH = 1600;
    static constexpr std::size_t SCREEN_HEIGHT = 1200;

    const std::string GLSL_VERSION = "#version 330";

    const DroneData INITIAL_DRONE_DATA = DroneData(glm::vec3(0.0, 0.1, 0.0), glm::vec3(0.0, 0.0, 0.0));
    const CameraData INITIAL_CAMERA_DATA = CameraData(glm::vec3(0.0, 1.0, 4.0), glm::vec3(0.0, 1.0, 3.0));

    /*
     * State.
     */
    std::shared_ptr<ViewerMode> viewer_mode = std::make_shared<ViewerMode>(ViewerMode::Telemetry);

    std::shared_ptr<DroneData> drone_data = std::make_shared<DroneData>(INITIAL_DRONE_DATA);
    std::shared_ptr<CameraData> camera_data = std::make_shared<CameraData>(INITIAL_CAMERA_DATA);

    /*
     * Data managers.
     */
    std::shared_ptr<GlfwManager> glfw_manager;
    std::shared_ptr<ImguiManager> imgui_manager;
    std::shared_ptr<OpenglManager> opengl_manager;
};


bool DroneViewer::init()
{
    glfw_manager = std::make_shared<GlfwManager>(SCREEN_WIDTH, SCREEN_HEIGHT, viewer_mode, drone_data);
    if (!glfw_manager->init()) { return false; }

    imgui_manager = std::make_shared<ImguiManager>(glfw_manager->get_window(), GLSL_VERSION, SCREEN_WIDTH, SCREEN_HEIGHT, viewer_mode, drone_data, camera_data);
    if (!imgui_manager->init()) { return false; }

    opengl_manager = std::make_shared<OpenglManager>(SCREEN_WIDTH, SCREEN_HEIGHT, drone_data, camera_data);
    if (!opengl_manager->init()) { return false; }

    return true;
}

bool DroneViewer::is_running() const
{
    return !glfw_manager->should_window_close();
}

void DroneViewer::process_frame()
{
    /*
     * Process input.
     */
    glfw_manager->process_input();

    /*
     * Render. Order between imgui_manager and opengl_manager is important.
     */
    imgui_manager->process_frame();
    imgui_manager->render();
    opengl_manager->process_frame();
    imgui_manager->render_draw_data();

    /*
     * Swap buffers and poll I/O events.
     */
    glfw_manager->swap_buffers();
    glfw_manager->poll_events();
}

#endif /* DRONE_VIEWER_HPP */
