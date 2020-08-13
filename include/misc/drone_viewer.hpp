#ifndef DRONE_VIEWER_HPP
#define DRONE_VIEWER_HPP

#include <filesystem>
#include <memory>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "camera.hpp"
#include "window_manager.hpp"
#include "ui_manager.hpp"
#include "lights.hpp"
#include "logger.hpp"
#include "graphics_manager.hpp"
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
     * Telemetry.
     */
    static constexpr std::size_t TELEMETRY_PACKET_LEN = 37;
    static constexpr char TELEMETRY_START_SYMBOL = '|';
    static constexpr char TELEMETRY_STOP_SYMBOL = '\n';
    static constexpr std::size_t TELEMETRY_FLOAT_CONVERSION_FACTOR = 1000;
    static constexpr std::size_t TELEMETRY_FLOAT_FORMAT_LEN = 5;
    const std::vector<std::size_t> TELEMETRY_ACCEL_OFFSETS = {1, 7, 13};
    const std::vector<std::size_t> TELEMETRY_ROT_RATE_OFFSETS = {19, 25, 31};

    static constexpr std::size_t SCREEN_WIDTH = 1200;
    // static constexpr std::size_t SCREEN_WIDTH = 900;
    static constexpr std::size_t SCREEN_HEIGHT = 900;

    static constexpr bool SHOW_DEMO_WINDOW = false;
    static constexpr bool SHOW_IMPLOT_DEMO_WINDOW = false;
    static constexpr bool SHOW_CAMERA_DATA_WINDOW = true;

    const std::string GLSL_VERSION = "#version 330";

    /*
     * Textures.
     */
    const fs::path texture_dir = "assets/textures";

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
    // General lights.
    static constexpr glm::vec3 diffuse_light_intensity = glm::vec3(0.5f);
    static constexpr glm::vec3 specular_light_intensity = glm::vec3(1.0f);

    static constexpr float light_attenuation_constant = 1.0f;
    static constexpr float light_attenuation_linear = 0.07f;
    static constexpr float light_attenuation_quadratic = 0.017f;

    // Point lights.
    const std::vector<glm::vec3> point_light_positions = {
        glm::vec3( 1.5f, 3.5f, 0.0f),
    };
    const std::vector<glm::vec3> point_light_colors = {
        glm::vec3(0.529f, 0.808f, 0.922f),
    };
    static constexpr float point_light_scale_factor = 0.2f;
    static constexpr glm::vec3 point_light_ambient_intensity = glm::vec3(0.8f);

    /*
     * Model settings.
     */
    static constexpr bool use_anti_aliasing = false;

    /*
     * Room.
     */
    static constexpr float room_scale_factor = 24.0f;
    static constexpr glm::vec3 room_dimensions = room_scale_factor * glm::vec3(1.0f, 0.5f, 1.0f);
    static constexpr glm::vec3 room_position = glm::vec3(0.0f, 0.0f, 0.0f);

    /*
     * Drone.
     */
    const fs::path drone_directory = "assets/models/drone";
    const fs::path drone_obj_path = drone_directory / "drone.obj";
    static constexpr bool drone_flip_textures = false;

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
    std::unique_ptr<WindowManager> window_manager;
    std::unique_ptr<UiManager> ui_manager;
    std::unique_ptr<GraphicsManager> graphics_manager;
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
        CAMERA_FRONT_HEADON);

    /*
     * Initialize data managers.
     */
    window_manager = std::make_unique<WindowManager>(
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        resource_manager.get(),
        viewer_mode.get(),
        drone_data.get(),
        camera.get(),
        serial_port.get(),
        use_anti_aliasing);
    if (!window_manager->init()) return false;

    ui_manager = std::make_unique<UiManager>(
        window_manager->get_window(),
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
    if (!ui_manager->init()) return false;

    graphics_manager = std::make_unique<GraphicsManager>(
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        room_dimensions,
        resource_manager.get(),
        drone_data.get(),
        camera.get(),
        use_anti_aliasing);
    if (!graphics_manager->init()) return false;

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
        room_scale_factor,
        room_dimensions,
        room_position);
    if (!room)
    {
        logger.log(LogLevel::fatal, "DroneViewer::init: Room not created\n");
        return false;
    }
    room->init();

    // Drone.
    drone = std::make_unique<Model>(
        drone_obj_path,
        drone_flip_textures,
        scene_lighting.get());
    drone->init();

    // Pass models to OpenGL manager.
    graphics_manager->pass_objects(
        scene_lighting.get(),
        room.get(),
        drone.get()
    );

    return true;
}

bool DroneViewer::is_running() const
{
    return !window_manager->should_window_close();
}

bool DroneViewer::process_frame()
{
    /*
     * Process input.
     */
    window_manager->process_input();
    if (*viewer_mode == ViewerMode::Telemetry)
        if (!telemetry_manager->process_telemetry()) return false;

    /*
     * Render. Order between ui_manager and graphics_manager is important.
     */
    camera->process_frame();
    ui_manager->process_frame();
    ui_manager->render();
    graphics_manager->process_frame();
    ui_manager->render_draw_data();

    /*
     * Swap buffers and poll I/O events.
     */
    window_manager->swap_buffers();
    window_manager->poll_events();

    return true;
}

#endif /* DRONE_VIEWER_HPP */
