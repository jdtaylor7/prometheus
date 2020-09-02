#ifndef WINDOW_MANAGER_HPP
#define WINDOW_MANAGER_HPP

#include <filesystem>
#include <functional>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <stb_image.h>

#include "callbacks.hpp"
#include "logger.hpp"
#include "serial_port.hpp"
#include "shared.hpp"
#include "timer_manager.hpp"
#include "viewer_mode.hpp"

namespace fs = std::filesystem;

class WindowManager
{
public:
    WindowManager(std::size_t width_,
                std::size_t height_,
                ResourceManager* resource_manager_,
                ViewerMode* viewer_mode_,
                DroneData* drone_data_,
                Camera* camera_,
                SerialPort* serial_port_,
                bool use_anti_aliasing_,
                glm::vec3 room_dimensions_,
                glm::vec3 room_position_) :
        screen_width(width_),
        screen_height(height_),
        rm(resource_manager_),
        viewer_mode(viewer_mode_),
        drone_data(drone_data_),
        camera(camera_),
        serial_port(serial_port_),
        use_anti_aliasing(use_anti_aliasing_),
        room_dimensions(room_dimensions_),
        room_position(room_position_)
    {
    }

    ~WindowManager();

    bool init();

    bool load_glad_loader();
    bool should_window_close() const;

    GLFWwindow* get_window() const;

    void process_input();
    void swap_buffers();
    void poll_events();
private:
    std::size_t screen_width;
    std::size_t screen_height;
    GLFWwindow* window;
    bool use_anti_aliasing;
    glm::vec3 room_dimensions;
    glm::vec3 room_position;

    ResourceManager* rm;
    DroneData* drone_data;
    Camera* camera;
    ViewerMode* viewer_mode;
    SerialPort* serial_port;

    fs::path icon_dir = "assets/icons";
    fs::path icon_16 = icon_dir / "icon_16.png";
    fs::path icon_32 = icon_dir / "icon_32.png";
    fs::path icon_48 = icon_dir / "icon_48.png";

    // Timers.
    std::unique_ptr<TimerManager> timer_manager;

    /*
     * Callback functions.
     */
    void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void cursor_callback(GLFWwindow* window, double xpos, double ypos);
};

bool WindowManager::init()
{
    /*
     * GLFW initialization and configuration.
     */
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    if (use_anti_aliasing)
        glfwWindowHint(GLFW_SAMPLES, 4);

    /*
     * GLFW window creation.
     */
    window = glfwCreateWindow(screen_width, screen_height, "Prometheus", NULL, NULL);
    if (!window)
    {
        logger.log(LogLevel::fatal, "WindowManager::init: Failed to create GLFW window\n");
        return false;
    }

    /*
     * Set window icon.
     */
    bool get_icons = true;
    if (!fs::exists(icon_16))
    {
        logger.log(LogLevel::error, "WindowManager::init: File does not exist: ", icon_16, '\n');
        get_icons = false;
    }
    if (!fs::exists(icon_32))
    {
        logger.log(LogLevel::error, "WindowManager::init: File does not exist: ", icon_32, '\n');
        get_icons = false;
    }
    if (!fs::exists(icon_48))
    {
        logger.log(LogLevel::error, "WindowManager::init: File does not exist: ", icon_48, '\n');
        get_icons = false;
    }

    if (get_icons)
    {
        GLFWimage icons[3];
        int num_channels;
        icons[0].pixels = stbi_load(icon_16.c_str(), &icons[0].width, &icons[0].height, &num_channels, 0);
        icons[1].pixels = stbi_load(icon_32.c_str(), &icons[1].width, &icons[1].height, &num_channels, 0);
        icons[2].pixels = stbi_load(icon_48.c_str(), &icons[2].width, &icons[2].height, &num_channels, 0);
        glfwSetWindowIcon(window, 3, icons);
        stbi_image_free(icons[0].pixels);
        stbi_image_free(icons[1].pixels);
        stbi_image_free(icons[2].pixels);
    }

    using namespace std::placeholders;

    // Prepare wrapper function objects for passing as callbacks. Solution based
    // off this Stackoverflow post: https://stackoverflow.com/a/19809787. Cannot
    // use other solutions as creating a lambda, creating a std::function
    // object, and making the member function static all do not suffice. This
    // post explains the problem in more depth:
    // https://stackoverflow.com/a/402385.
    FramebufferCallback<void(GLFWwindow*, int, int)>::func = std::bind(&WindowManager::framebuffer_size_callback, this, _1, _2, _3);
    CursorCallback<void(GLFWwindow*, double, double)>::func = std::bind(&WindowManager::cursor_callback, this, _1, _2, _3);

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, static_cast<void(*)(GLFWwindow*, int, int)>(FramebufferCallback<void(GLFWwindow*, int, int)>::callback));
    glfwSetCursorPosCallback(window, static_cast<void(*)(GLFWwindow*, double, double)>(CursorCallback<void(GLFWwindow*, double, double)>::callback));

    /*
     * Load OpenGL function pointers.
     */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        logger.log(LogLevel::fatal, "Failed to initialize GLAD\n");
        return false;
    }

    /*
     * Set up timers.
     */
    timer_manager = std::make_unique<TimerManager>();

    using namespace std::chrono_literals;
    timer_manager->register_timer(TimerName::ComScanTimer, 200ms);
    timer_manager->register_timer(TimerName::ComConnectTimer, 200ms);
    timer_manager->register_timer(TimerName::ComReadTimer, 200ms);

    return true;
}

WindowManager::~WindowManager()
{
    glfwTerminate();
}

bool WindowManager::load_glad_loader()
{
    return gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
}

bool WindowManager::should_window_close() const
{
    return glfwWindowShouldClose(window);
}

GLFWwindow* WindowManager::get_window() const
{
    return window;
}

void WindowManager::process_input()
{
    /*
     * Exit application.
     */
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    /*
     * Enter telemetry mode.
     */
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    {
        if (rm)
        {
            std::lock_guard<std::mutex> g(rm->viewer_mode_mutex);
            *viewer_mode = ViewerMode::Telemetry;
        }
        else
        {
            logger.log(LogLevel::error, "WindowManager::process_input: \
                rm pointer is null\n");
        }
    }
    /*
     * Enter edit mode.
     */
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        if (rm)
        {
            std::lock_guard<std::mutex> g(rm->viewer_mode_mutex);
            *viewer_mode = ViewerMode::Edit;
        }
        else
        {
            logger.log(LogLevel::error, "WindowManager::process_input: \
                rm pointer is null\n");
        }
    }

    if (*viewer_mode == ViewerMode::Telemetry)
    {
        /*
         * Show cursor, disables camera control.
         */
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        /*
         * Scan for serial devices.
         */
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            if (timer_manager->is_finished(TimerName::ComScanTimer))
            {
                if (serial_port)
                    serial_port->find_ports();
                else
                    logger.log(LogLevel::error, "WindowManager::process_input: \
                        serial_port is null\n");
                timer_manager->start_timer(TimerName::ComScanTimer);
            }
        }

        /*
         * Connect to selected serial device.
         */
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        {
            if (timer_manager->is_finished(TimerName::ComConnectTimer))
            {
                if (!serial_port)
                    logger.log(LogLevel::error, "WindowManager::process_input: \
                        serial_port is null\n");

                if (serial_port && !serial_port->get_available_ports().empty())
                {
                    serial_port->open(serial_port->get_available_ports()[0]);
                    serial_port->config();
                }
                timer_manager->start_timer(TimerName::ComConnectTimer);
            }
        }

        /*
         * Start/stop reading from connected serial device.
         */
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            if (timer_manager->is_finished(TimerName::ComReadTimer))
            {
                if (serial_port)
                {
                    if (serial_port->is_reading())
                        serial_port->stop_reading();
                    else
                        serial_port->start_reading();
                }
                else
                {
                    logger.log(LogLevel::error, "WindowManager::process_input: \
                        serial_port is null\n");
                }
                timer_manager->start_timer(TimerName::ComReadTimer);
            }
        }
    }

    if (*viewer_mode == ViewerMode::Edit)
    {
        /*
         * Disable cursor to allow camera control.
         */
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        /*
         * Edit drone position.
         */
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            std::lock_guard<std::mutex> g(rm->drone_data_mutex);
            drone_data->position.y += 0.05f;
            if (drone_data->position.y > room_dimensions.y - (DRONE_OFFSET_TOP / 2))
                drone_data->position.y = room_dimensions.y - (DRONE_OFFSET_TOP / 2);
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            std::lock_guard<std::mutex> g(rm->drone_data_mutex);
            drone_data->position.y -= 0.05f;
            if (drone_data->position.y < room_position.y + (DRONE_OFFSET_BOT / 2))
                drone_data->position.y = room_position.y + (DRONE_OFFSET_BOT / 2);
        }

        /*
         * Reset camera and drone.
         */
        if (!camera)
            logger.log(LogLevel::error, "WindowManager::process_input: \
                camera is null\n");
        if (!rm)
            logger.log(LogLevel::error, "WindowManager::process_input: \
                rm is null\n");
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            if (rm)
            {
                std::lock_guard<std::mutex> g(rm->drone_data_mutex);
                *drone_data = INITIAL_DRONE_DATA;
            }

            if (camera && rm)
            {
                std::lock_guard<std::mutex> g(rm->camera_data_mutex);
                camera->set_position(CAMERA_POSITION_HEADON);
                camera->set_front(CAMERA_FRONT_HEADON);
                camera->set_pitch(CAMERA_PITCH_HEADON);
                camera->set_yaw(CAMERA_YAW_HEADON);
            }
        }

        if (camera)
            camera->update_position(window);
    }
}

void WindowManager::swap_buffers()
{
    glfwSwapBuffers(window);
}

void WindowManager::poll_events()
{
    glfwPollEvents();
}

/*
 * Callback functions.
 */
void WindowManager::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void WindowManager::cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (*viewer_mode == ViewerMode::Edit)
        if (camera)
            camera->update_angle(xpos, ypos);
        else
            logger.log(LogLevel::error, "WindowManager::cursor_callback: \
                camera pointer null\n");
}

#endif /* WINDOW_MANAGER_HPP */
