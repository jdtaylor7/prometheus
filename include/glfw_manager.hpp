#ifndef GLFW_MANAGER_HPP
#define GLFW_MANAGER_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

/*
 * Callback functions.
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    // camera.update_angle(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // camera.update_pov(yoffset);
}

class GlfwManager
{
public:
    GlfwManager(std::size_t width, std::size_t height);
    ~GlfwManager();

    bool did_window_creation_fail() const;
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
    bool window_creation_failed = false;
};

GlfwManager::GlfwManager(std::size_t width, std::size_t height) :
    screen_width(width), screen_height(height)
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
        window_creation_failed = true;
    }

    if (!window_creation_failed)
    {
        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);
    }
}

GlfwManager::~GlfwManager()
{
    glfwTerminate();
}

bool GlfwManager::did_window_creation_fail() const
{
    return window_creation_failed;
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

    // if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
    // {
    //     printer.print_camera_details();
    // }
    //
    // if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    //     drone_pos.y += 0.003f;
    //
    // if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    //     drone_pos.y -= 0.003f;
    //
    // camera.update_pos(window);
}

void GlfwManager::swap_buffers()
{
    glfwSwapBuffers(window);
}

void GlfwManager::poll_events()
{
    glfwPollEvents();
}

#endif /* GLFW_MANAGER_HPP */
