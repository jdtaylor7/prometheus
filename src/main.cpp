#include <iostream>
#include <string>

#include <glad/glad.h>

#include "camera.hpp"
#include "glfw_manager.hpp"
#include "imgui_manager.hpp"
#include "opengl_manager.hpp"
#include "resource_manager.hpp"
#include "shader.hpp"
#include "vertex_data.hpp"

/*
 * Global data.
 */
constexpr std::size_t SCREEN_WIDTH = 1600;
constexpr std::size_t SCREEN_HEIGHT = 1200;

/*
 * Main function.
 */
int main()
{
    /*
     * GLFW initialization.
     */
    GlfwManager glfw_manager(SCREEN_WIDTH, SCREEN_HEIGHT);
    if (glfw_manager.did_window_creation_fail())
    {
        std::cout << "Failed to create GLFW window\n";
        return -1;
    }

    /*
     * Load OpenGL function pointers.
     */
    if (!glfw_manager.load_glad_loader())
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }
    ImguiManager imgui_manager(glfw_manager.get_window(), "#version 330", SCREEN_WIDTH, SCREEN_HEIGHT);

    OpenglManager opengl_manager(SCREEN_WIDTH, SCREEN_HEIGHT);

    /*
     * Render loop.
     */
    while (!glfw_manager.should_window_close())
    {
        // Update ImGUI windows.
        // imgui_manager.update_drone_data(drone_x, 0.0, 0.0, 0.0, 0.0, 0.0);
        imgui_manager.execute_frame();
        imgui_manager.render();

        /*
         * Process input.
         */
        glfw_manager.process_input();

        /*
         * Render.
         */
        opengl_manager.process_frame();
        imgui_manager.render_draw_data();

        /*
         * Swap buffers and poll I/O events.
         */
        glfw_manager.swap_buffers();
        glfw_manager.poll_events();
    }

    return 0;
}
