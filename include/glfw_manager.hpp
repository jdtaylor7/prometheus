#ifndef GLFW_MANAGER_HPP
#define GLFW_MANAGER_HPP

#include <functional>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "callbacks.hpp"
#include "shared.hpp"
#include "viewer_mode.hpp"

class GlfwManager
{
public:
    GlfwManager(std::size_t width_,
                std::size_t height_,
                ResourceManager* resource_manager_,
                ViewerMode* viewer_mode_,
                DroneData* drone_data_,
                Camera* camera_) :
    screen_width(width_),
    screen_height(height_),
    rm(resource_manager_),
    viewer_mode(viewer_mode_),
    drone_data(drone_data_),
    camera(camera_)
    {}

    ~GlfwManager();

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

    ResourceManager* rm;

    DroneData* drone_data;
    Camera* camera;
    ViewerMode* viewer_mode;

    /*
     * Callback functions.
     */
    void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void cursor_callback(GLFWwindow* window, double xpos, double ypos);
    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
};

bool GlfwManager::init()
{
    /*
     * GLFW initialization and configuration.
     */
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /*
     * GLFW window creation.
     */
    window = glfwCreateWindow(screen_width, screen_height, "Drone Viewer", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window\n";
        return false;
    }

    using namespace std::placeholders;

    // Prepare wrapper function objects for passing as callbacks. Solution based
    // off this Stackoverflow post: https://stackoverflow.com/a/19809787. Cannot
    // use other solutions like creating a lambda, creating a std::function
    // object, and making the member function static all do not suffice. This
    // post explains the problem in even more depth:
    // https://stackoverflow.com/a/402385.
    FramebufferCallback<void(GLFWwindow*, int, int)>::func = std::bind(&GlfwManager::framebuffer_size_callback, this, _1, _2, _3);
    CursorCallback<void(GLFWwindow*, double, double)>::func = std::bind(&GlfwManager::cursor_callback, this, _1, _2, _3);
    ScrollCallback<void(GLFWwindow*, double, double)>::func = std::bind(&GlfwManager::scroll_callback, this, _1, _2, _3);

    glfwMakeContextCurrent(window);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetFramebufferSizeCallback(window, static_cast<void(*)(GLFWwindow*, int, int)>(FramebufferCallback<void(GLFWwindow*, int, int)>::callback));
    glfwSetCursorPosCallback(window, static_cast<void(*)(GLFWwindow*, double, double)>(CursorCallback<void(GLFWwindow*, double, double)>::callback));
    glfwSetScrollCallback(window, static_cast<void(*)(GLFWwindow*, double, double)>(ScrollCallback<void(GLFWwindow*, double, double)>::callback));

    /*
     * Load OpenGL function pointers.
     */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return false;
    }

    return true;
}

GlfwManager::~GlfwManager()
{
    glfwTerminate();
}

bool GlfwManager::load_glad_loader()
{
    return gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
}

bool GlfwManager::should_window_close() const
{
    return glfwWindowShouldClose(window);
}

GLFWwindow* GlfwManager::get_window() const
{
    return window;
}

void GlfwManager::process_input()
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

    // TODO Testing.
    if (*viewer_mode == ViewerMode::Edit)
    {
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            drone_data->position.y += 0.003f;

        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            drone_data->position.y -= 0.003f;

        camera->update_position(window);
    }
}

void GlfwManager::swap_buffers()
{
    glfwSwapBuffers(window);
}

void GlfwManager::poll_events()
{
    glfwPollEvents();
}

/*
 * Callback functions.
 */
void GlfwManager::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void GlfwManager::cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (*viewer_mode == ViewerMode::Edit)
        camera->update_angle(xpos, ypos);
}

void GlfwManager::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (*viewer_mode == ViewerMode::Edit)
        camera->update_pov(yoffset);
}

#endif /* GLFW_MANAGER_HPP */
