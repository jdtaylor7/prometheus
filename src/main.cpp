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

/*
 * Global data.
 */
constexpr std::size_t SCREEN_WIDTH = 1600;
constexpr std::size_t SCREEN_HEIGHT = 1200;

auto viewer_mode = std::make_shared<ViewerMode>(ViewerMode::Telemetry);

auto drone_data = std::make_shared<DroneData>(glm::vec3(0.0, 0.1, 0.0),
                                              glm::vec3(0.0, 0.0, 0.0));
auto camera_data = std::make_shared<CameraData>(glm::vec3(0.0, 1.0, 4.0),
                                                glm::vec3(0.0, 1.0, 3.0));

/*
 * Main function.
 */
int main()
{
    /*
     * GLFW initialization.
     */
    GlfwManager glfw_manager(SCREEN_WIDTH, SCREEN_HEIGHT, drone_data, viewer_mode);
    if (!glfw_manager.init()) { return -1; }

    ImguiManager imgui_manager(glfw_manager.get_window(), "#version 330", SCREEN_WIDTH, SCREEN_HEIGHT, viewer_mode, drone_data, camera_data);
    if (!imgui_manager.init()) { return -1; }

    OpenglManager opengl_manager(SCREEN_WIDTH, SCREEN_HEIGHT, drone_data, camera_data);
    if (!opengl_manager.init()) { return -1; }

    /*
     * Render loop.
     */
    while (!glfw_manager.should_window_close())
    {
        // Update ImGUI windows.
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
