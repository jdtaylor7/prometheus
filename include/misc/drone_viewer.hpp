#ifndef DRONE_VIEWER_HPP
#define DRONE_VIEWER_HPP

#include <iostream>
#include <filesystem>
#include <memory>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "camera.hpp"
#include "glfw_manager.hpp"
#include "imgui_manager.hpp"
#include "lights.hpp"
#include "opengl_manager.hpp"
#include <quad.hpp>
#include "resource_manager.hpp"
#include "serial_port.hpp"
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

    // static constexpr std::size_t SCREEN_WIDTH = 1600;
    // static constexpr std::size_t SCREEN_HEIGHT = 1200;
    static constexpr std::size_t SCREEN_WIDTH = 1200;
    static constexpr std::size_t SCREEN_HEIGHT = 900;

    static constexpr bool SHOW_DEMO_WINDOW = false;
    static constexpr bool SHOW_IMPLOT_DEMO_WINDOW = false;
    static constexpr bool SHOW_CAMERA_DATA_WINDOW = true;

    static constexpr glm::vec3 room_dimensions = glm::vec3(24.0f, 12.0f, 24.0f);

    const std::string GLSL_VERSION = "#version 330";

    const fs::path texture_dir = "assets/textures";

    // Individual textures. TODO remove unnecessary ones.
    const fs::path container_texture_path = texture_dir / "container.jpg";
    const fs::path face_texture_path = texture_dir / "awesomeface.png";
    const fs::path wall_texture_path = texture_dir / "wall.jpg";
    const fs::path box_diffuse_texture_path = texture_dir / "box_specular_map.jpg";
    const fs::path box_specular_texture_path = texture_dir / "box_diffuse_map.jpg";

    // Texture family directories.
    const fs::path tile_floor_texture_dir = texture_dir / "tile_floor";
    const fs::path scifi_wall_texture_dir = texture_dir / "scifi_wall";

    // Texture families.
    const fs::path tile_floor_texture_diff = tile_floor_texture_dir / "diffuse.png";
    const fs::path tile_floor_texture_spec = tile_floor_texture_dir / "specular.png";
    const fs::path scifi_wall_texture_diff = scifi_wall_texture_dir / "diffuse.png";
    const fs::path scifi_wall_texture_spec = scifi_wall_texture_dir / "specular.png";

    /*
     * Lights.
     */
    glm::vec3 diffuse_light_intensity = glm::vec3(0.5f);
    glm::vec3 specular_light_intensity = glm::vec3(1.0f);

    float light_attenuation_constant = 1.0f;
    float light_attenuation_linear = 0.07f;
    float light_attenuation_quadratic = 0.017f;

    // Point lights.
    std::vector<glm::vec3> point_light_positions = {
        glm::vec3( 1.5f, 3.5f, 0.0f),
    };
    std::vector<glm::vec3> point_light_colors = {
        glm::vec3(0.529f, 0.808f, 0.922f),
    };
    float point_light_scale_factor = 0.2f;
    glm::vec3 point_light_ambient_intensity = glm::vec3(0.8f);

    /*
     * Room.
     */
    float room_scale_factor = 24.0f;

    /*
     * Drone.
     */
    const fs::path drone_directory = "assets/models/drone";
    const fs::path drone_obj_path = drone_directory / "drone.obj";
    bool drone_flip_textures = false;

#ifdef OS_LINUX
    std::unique_ptr<const LinuxSerialPortConfig> linux_serial_cfg =
        std::make_unique<const LinuxSerialPortConfig>(
            LibSerial::BaudRate::BAUD_9600,
            LibSerial::CharacterSize::CHAR_SIZE_8,
            LibSerial::FlowControl::FLOW_CONTROL_NONE,
            LibSerial::Parity::PARITY_NONE,
            LibSerial::StopBits::STOP_BITS_1);
#endif

    /*
     * Shared state.
     */
    std::unique_ptr<ViewerMode> viewer_mode;
    std::unique_ptr<DroneData> drone_data;
    std::unique_ptr<Camera> camera;
    std::shared_ptr<BoundedBuffer<char>> telemetry_buffer;

    /*
     * OpenGL models.
     */
    std::vector<std::shared_ptr<PointLight>> point_lights;
    std::unique_ptr<SceneLighting> scene_lighting;
    std::unique_ptr<Room> room;
    std::unique_ptr<Model> drone;
    std::unique_ptr<Quad> quad;

    /*
     * Synchronization constructs.
     */
    std::unique_ptr<ResourceManager> resource_manager;

    /*
     * Communications interfaces.
     */
    std::unique_ptr<SerialPort> serial_port;

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
     * Create telemetry buffer.
     */
    telemetry_buffer =
        std::make_shared<BoundedBuffer<char>>((TELEMETRY_PACKET_LEN * 2) - 1);

    /*
     * Initialize communications interfaces.
     */
#ifdef OS_CYGWIN
    serial_port = std::make_unique<SerialPort>(telemetry_buffer);
#elif OS_LINUX
    serial_port = std::make_unique<SerialPort>(
        telemetry_buffer,
        linux_serial_cfg.get());
#endif

    /*
     * Attempt to auto-open a port if only one is available. Else, do nothing
     * since ports can easily be opened once the application is running.
     */
    serial_port->auto_open();

    /*
     * Initialize state.
     */
    viewer_mode = std::make_unique<ViewerMode>(ViewerMode::Telemetry);
    drone_data = std::make_unique<DroneData>(INITIAL_DRONE_DATA);
    camera = std::make_unique<Camera>(
        resource_manager.get(),
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        room_dimensions,
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
        serial_port.get());
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
        serial_port.get(),
        SHOW_DEMO_WINDOW,
        SHOW_IMPLOT_DEMO_WINDOW,
        SHOW_CAMERA_DATA_WINDOW);
    if (!imgui_manager->init()) return false;

    opengl_manager = std::make_unique<OpenglManager>(
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        room_dimensions,
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
        serial_port.get(),
        drone_data.get(),
        resource_manager.get(),
        telemetry_buffer);
    if (!telemetry_manager->init()) return false;

    /*
     * Initialize OpenGL models.
     */
    // Point lights.
    assert(point_light_positions.size() == point_light_colors.size());
    for (std::size_t i = 0; i < point_light_positions.size(); i++)
    {
        auto point_light = std::make_shared<PointLight>(
            point_light_positions[i],
            point_light_colors[i],
            point_light_scale_factor,
            point_light_ambient_intensity,
            diffuse_light_intensity,
            specular_light_intensity,
            light_attenuation_constant,
            light_attenuation_linear,
            light_attenuation_quadratic);
        point_light->init();
        point_lights.push_back(point_light);
    }

    // Scene lighting.
    scene_lighting = std::make_unique<SceneLighting>(
        nullptr,
        point_lights,
        nullptr
    );

    // Room.
    room = std::make_unique<Room>(
        tile_floor_texture_diff,
        tile_floor_texture_spec,
        tile_floor_texture_diff,
        tile_floor_texture_spec,
        scifi_wall_texture_diff,
        scifi_wall_texture_spec,
        scene_lighting.get(),
        room_scale_factor);
    if (!room)
    {
        std::cout << "DroneViewer::init: Room not created";
        return false;
    }
    room->init();

    // Drone.
    drone = std::make_unique<Model>(
        drone_obj_path,
        drone_flip_textures,
        scene_lighting.get());
    drone->init();

    // Quad.
    quad = std::make_unique<Quad>();
    quad->init();

    // Pass models to OpenGL manager.
    opengl_manager->pass_objects(
        scene_lighting.get(),
        room.get(),
        drone.get(),
        quad.get()
    );

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
