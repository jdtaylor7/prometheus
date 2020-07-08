#ifndef DRONE_VIEWER_HPP
#define DRONE_VIEWER_HPP

#include <iostream>
#include <memory>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "camera.hpp"
#include "com_port.hpp"
#include "glfw_manager.hpp"
#include "imgui_manager.hpp"
#include "opengl_manager.hpp"
#include "resource_manager.hpp"
#include "shader.hpp"
#include "shared.hpp"
#include "telemetry_manager.hpp"
#include "vertex_data.hpp"
#include "viewer_mode.hpp"

class DroneViewer
{
public:
    bool init();
    bool is_running() const;

    bool process_frame();
private:
    /*
     * Constants.
     */
    static constexpr std::size_t TELEMETRY_PACKET_LEN = 37;
    static constexpr char TELEMETRY_START_SYMBOL = '|';
    static constexpr char TELEMETRY_STOP_SYMBOL = '\n';
    static constexpr std::size_t TELEMETRY_FLOAT_CONVERSION_FACTOR = 1000;
    static constexpr std::size_t TELEMETRY_FLOAT_FORMAT_LEN = 5;
    const std::vector<std::size_t> TELEMETRY_ACCEL_OFFSETS = {1, 7, 13};
    const std::vector<std::size_t> TELEMETRY_ROT_RATE_OFFSETS = {19, 25, 31};

    static constexpr std::size_t SCREEN_WIDTH = 1600;
    static constexpr std::size_t SCREEN_HEIGHT = 1200;

    static constexpr bool SHOW_DEMO_WINDOW = false;
    static constexpr bool SHOW_IMPLOT_DEMO_WINDOW = false;
    static constexpr bool SHOW_CAMERA_DATA_WINDOW = true;

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
     * Communications interfaces.
     */
    std::unique_ptr<ComPort> com_port;

    /*
     * Data managers.
     */
    std::unique_ptr<GlfwManager> glfw_manager;
    std::unique_ptr<ImguiManager> imgui_manager;
    std::unique_ptr<OpenglManager> opengl_manager;
    std::unique_ptr<TelemetryManager> telemetry_manager;
};

bool DroneViewer::init()
{
    /*
     * Initialize synchronization constructs.
     */
    resource_manager = std::make_unique<ResourceManager>();

    /*
     * Initialize communications interfaces.
     */
    com_port = std::make_unique<ComPort>(
        TELEMETRY_PACKET_LEN,
        TELEMETRY_START_SYMBOL,
        TELEMETRY_STOP_SYMBOL
    );
    // It's fine if this fails. It can be controlled via the UI later.
    if (!com_port)
        std::cout << "com_port is null\n";
    std::vector<unsigned int> available_ports = com_port->find_ports();
    if (!available_ports.empty())
    {
        com_port->connect(available_ports[0]);
        com_port->init();
    }

    /*
     * Initialize state.
     */
    viewer_mode = std::make_unique<ViewerMode>(ViewerMode::Telemetry);
    drone_data = std::make_unique<DroneData>(INITIAL_DRONE_DATA);
    camera = std::make_unique<Camera>(
        resource_manager.get(),
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        ROOM_SIZE / 2,
        ROOM_SIZE,
        CAMERA_POSITION_HEADON,
        CAMERA_TARGET_HEADON);

    /*
     * Initialize data managers.
     */
    glfw_manager = std::make_unique<GlfwManager>(
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        resource_manager.get(),
        viewer_mode.get(),
        drone_data.get(),
        camera.get(),
        com_port.get());
    if (!glfw_manager->init()) return false;

    imgui_manager = std::make_unique<ImguiManager>(
        glfw_manager->get_window(),
        GLSL_VERSION,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        resource_manager.get(),
        viewer_mode.get(),
        drone_data.get(),
        camera.get(),
        com_port.get(),
        SHOW_DEMO_WINDOW,
        SHOW_IMPLOT_DEMO_WINDOW,
        SHOW_CAMERA_DATA_WINDOW);
    if (!imgui_manager->init()) return false;

    opengl_manager = std::make_unique<OpenglManager>(
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        ROOM_SIZE,
        resource_manager.get(),
        drone_data.get(),
        camera.get());
    if (!opengl_manager->init()) return false;

    telemetry_manager = std::make_unique<TelemetryManager>(
        TELEMETRY_PACKET_LEN,
        TELEMETRY_START_SYMBOL,
        TELEMETRY_STOP_SYMBOL,
        TELEMETRY_FLOAT_CONVERSION_FACTOR,
        TELEMETRY_FLOAT_FORMAT_LEN,
        TELEMETRY_ACCEL_OFFSETS,
        TELEMETRY_ROT_RATE_OFFSETS,
        com_port.get(),
        drone_data.get(),
        resource_manager.get());
    if (!telemetry_manager->init()) return false;

    return true;
}

bool DroneViewer::is_running() const
{
    return !glfw_manager->should_window_close();
}

bool DroneViewer::process_frame()
{
    /*
     * Process input.
     */
    glfw_manager->process_input();
    if (*viewer_mode == ViewerMode::Telemetry)
        if (!telemetry_manager->process_telemetry()) return false;

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

    return true;
}

#endif /* DRONE_VIEWER_HPP */
