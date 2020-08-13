#ifndef WINDOW_MANAGER_HPP
#define WINDOW_MANAGER_HPP

#include <functional>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "callbacks.hpp"
#include "logger.hpp"
#include "serial_port.hpp"
#include "shared.hpp"
#include "timer_manager.hpp"
#include "viewer_mode.hpp"

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
    window = glfwCreateWindow(screen_width, screen_height, "Drone Viewer", NULL, NULL);
    if (!window)
    {
        logger.log(LogLevel::fatal, "Failed to create GLFW window\n");
        return false;
    }

    using namespace std::placeholders;

    // Prepare wrapper function objects for passing as callbacks. Solution based
    // off this Stackoverflow post: https://stackoverflow.com/a/19809787. Cannot
    // use other solutions as creating a lambda, creating a std::function
    // object, and making the member function static all do not suffice. This
    // post explains the problem in even more depth:
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
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    {
        std::lock_guard<std::mutex> g(rm->viewer_mode_mutex);
        *viewer_mode = ViewerMode::Telemetry;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        std::lock_guard<std::mutex> g(rm->viewer_mode_mutex);
        *viewer_mode = ViewerMode::Edit;
    }

    if (*viewer_mode == ViewerMode::Telemetry)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            if (timer_manager->is_finished(TimerName::ComScanTimer))
            {
                serial_port->find_ports();
                timer_manager->start_timer(TimerName::ComScanTimer);
            }
        }

        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        {
            if (timer_manager->is_finished(TimerName::ComConnectTimer))
            {
                if (!serial_port->get_available_ports().empty())
                {
                    serial_port->open(serial_port->get_available_ports()[0]);
                    serial_port->config();
                }
                timer_manager->start_timer(TimerName::ComConnectTimer);
            }
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            if (timer_manager->is_finished(TimerName::ComReadTimer))
            {
                if (serial_port->is_reading())
                    serial_port->stop_reading();
                else
                    serial_port->start_reading();
                timer_manager->start_timer(TimerName::ComReadTimer);
            }
        }
    }

    if (*viewer_mode == ViewerMode::Edit)
    {
        // TODO Testing.
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            std::lock_guard<std::mutex> g(rm->drone_data_mutex);
            drone_data->position.y += 0.05f;
            if (drone_data->position.y > room_dimensions.y - (DRONE_OFFSET_TOP / 2))
                drone_data->position.y = room_dimensions.y - (DRONE_OFFSET_TOP / 2);
        }

        // TODO Testing.
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            std::lock_guard<std::mutex> g(rm->drone_data_mutex);
            drone_data->position.y -= 0.05f;
            if (drone_data->position.y < room_position.y + (DRONE_OFFSET_BOT / 2))
                drone_data->position.y = room_position.y + (DRONE_OFFSET_BOT / 2);
        }

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            {
                std::lock_guard<std::mutex> g(rm->drone_data_mutex);
                *drone_data = INITIAL_DRONE_DATA;
            }

            {
                std::lock_guard<std::mutex> g(rm->camera_data_mutex);
                camera->set_position(CAMERA_POSITION_HEADON);
                camera->set_front(CAMERA_FRONT_HEADON);
                camera->set_pitch(CAMERA_PITCH_HEADON);
                camera->set_yaw(CAMERA_YAW_HEADON);
            }
        }
        // TODO
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        {
            {
                std::lock_guard<std::mutex> g(rm->camera_data_mutex);
                camera->set_position(CAMERA_POSITION_OVERHEAD);
                camera->set_front(CAMERA_FRONT_OVERHEAD);
                camera->set_pitch(CAMERA_PITCH_OVERHEAD);
                camera->set_yaw(CAMERA_YAW_OVERHEAD);
            }
        }

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
        camera->update_angle(xpos, ypos);
}

#endif /* WINDOW_MANAGER_HPP */
