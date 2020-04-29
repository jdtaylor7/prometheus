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

    static constexpr float ROOM_SIZE = 10.0f;

    const std::string GLSL_VERSION = "#version 330";

    /*
     * Shared state.
     */
    std::unique_ptr<ViewerMode> viewer_mode;
    std::unique_ptr<DroneData> drone_data;
    std::unique_ptr<Camera> camera;

    /*
     * Synchronization constructs.
     */
    std::unique_ptr<ResourceManager> resource_manager;

    /*
     * Data managers.
     */
    std::unique_ptr<GlfwManager> glfw_manager;
    std::unique_ptr<ImguiManager> imgui_manager;
    std::unique_ptr<OpenglManager> opengl_manager;
};

bool DroneViewer::init()
{
    /*
     * Initialize synchronization constructs.
     */
    resource_manager = std::make_unique<ResourceManager>();

    /*
     * Initialize state.
     */
    viewer_mode = std::make_unique<ViewerMode>(ViewerMode::Telemetry);
    drone_data = std::make_unique<DroneData>(INITIAL_DRONE_DATA);
    // camera_data = std::make_unique<CameraData>(INITIAL_CAMERA_DATA);
    camera = std::make_unique<Camera>(
        resource_manager.get(),
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        ROOM_SIZE / 2,
        ROOM_SIZE,
        INITIAL_CAMERA_POSITION,
        INITIAL_CAMERA_TARGET);

    /*
     * Initialize data managers.
     */
    glfw_manager = std::make_unique<GlfwManager>(
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        resource_manager.get(),
        viewer_mode.get(),
        drone_data.get(),
        camera.get());
    if (!glfw_manager->init()) return false;

    imgui_manager = std::make_unique<ImguiManager>(
        glfw_manager->get_window(),
        GLSL_VERSION,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        resource_manager.get(),
        viewer_mode.get(),
        drone_data.get(),
        camera.get());
    if (!imgui_manager->init()) return false;

    opengl_manager = std::make_unique<OpenglManager>(
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        ROOM_SIZE,
        resource_manager.get(),
        drone_data.get(),
        camera.get());
    if (!opengl_manager->init()) return false;

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
    camera->process_frame();
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
