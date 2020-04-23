#ifndef IMGUI_MANAGER_HPP
#define IMGUI_MANAGER_HPP

#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

struct ImguiWindowSettings
{
    float width;
    float height;
    float xpos;
    float ypos;

    float bottom() { return ypos + height };
};

class ImguiManager
{
public:
    ImguiManager(GLFWwindow* window, const std::string& glsl_version);
    ~ImguiManager();

    void execute_frame();
    void render();

    void update_drone_data(float dx, float dy, float dz,
        float roll, float pitch, float yaw);
    void camera_data(float cx, float cy, float cz,
        float tx, float ty, float tz);
    void update_queue_data(unsigned int p, unsigned int c);
private:
    const ImGuiWindowFlags imgui_window_flags = ImGuiWindowFlags_NoResize |
                                                ImGuiWindowFlags_NoCollapse;

    ImGuiIO& io;

    bool show_demo_window;
    bool show_another_window;
    const ImVec4 clear_color(0.45f, 0.55f, 0.60f, 1.00f);

    constexpr float WINDOW_BUF = 20.0f;

    ImGuiWindowSettings fps{93.0, 32.0, WINDOW_BUF, WINDOW_BUF};
    ImGuiWindowSettings mode{165.0, 123.0, SCREEN_WIDTH - mode.width - WINDOW_BUF, WINDOW_BUF};
    ImGuiWindowSettings controls{163.0, 82.0, SCREEN_WIDTH - controls.width, mode.bottom() + WINDOW_BUF};
    ImGuiWindowSettings drone{121.0, 167.0, WINDOW_BUF, fps.bottom() + WINDOW_BUF};
    ImGuiWindowSettings camera{121.0, 167.0, WINDOW_BUF, drone.bottom() + WINDOW_BUF};
    ImGuiWindowSettings queue{171.0, 65.0, WINDOW_BUF, camera.bottom() + WINDOW_BUF};

    float drone_x = 0.000f;
    float drone_y = 0.000f;
    float drone_z = 0.000f;
    float drone_roll = 0.000f;
    float drone_pitch = 0.000f;
    float drone_yaw = 0.000f;

    float camera_x = 0.000f;
    float camera_y = 0.000f;
    float camera_z = 0.000f;
    float target_x = 0.000f;
    float target_y = 0.000f;
    float target_z = 0.000f;

    unsigned int producer_n = 0;
    unsigned int consumer_n = 0;
};

ImguiManager::ImguiManager(GLFWwindow* window, const std::string& glsl_version)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = ImGui::GetIO();

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version.c_str());

    show_demo_window = true;
    show_another_window = true;
}

ImguiManager::~ImguiManager()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImguiManager::execute_frame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (show_demo_window)
    {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    // FPS window.
    ImGui::SetNextWindowSize(ImVec2(fps.width, fps.height), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(fps.xpos, fps.ypos), ImGuiCond_Always);
    {
        ImGui::Begin("FPS", NULL, imgui_window_flags | ImGuiWindowFlags_NoTitleBar);
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::End();
    }

    // Mode window.
    ImGui::SetNextWindowSize(ImVec2(mode_window_width, mode_window_height), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(mode_window_xpos, mode_window_ypos), ImGuiCond_Always);
    {
        ImGui::Begin("Application Mode", NULL, imgui_window_flags);

        static int e = 0;
        ImGui::RadioButton("GUI (g)", &e, 0);
        ImGui::RadioButton("Simulate (s)", &e, 1);
        ImGui::RadioButton("Camera control (c)", &e, 2);
        ImGui::RadioButton("Drone control (d)", &e, 3);

        ImGui::End();
    }

    // Controls window.
    ImGui::SetNextWindowSize(ImVec2(controls_window_width, controls_window_height), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(controls_window_xpos, controls_window_ypos), ImGuiCond_Always);
    {
        ImGui::Begin("Simulation Controls", NULL, imgui_window_flags);

        ImGui::BulletText("Start/Stop (space)");
        ImGui::BulletText("Pause (p)");
        ImGui::BulletText("Reset (r)");

        ImGui::End();
    }

    // Drone data window.
    ImGui::SetNextWindowSize(ImVec2(drone_window_width, drone_window_height), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(drone_window_xpos, drone_window_ypos), ImGuiCond_Always);
    {
        ImGui::Begin("Drone Data", NULL, imgui_window_flags);

        ImGui::Text("Position");
        ImGui::BulletText("x:     %.3f", drone_x);
        ImGui::BulletText("y:     %.3f", drone_y);
        ImGui::BulletText("z:     %.3f", drone_z);

        ImGui::Text("Angle");
        ImGui::BulletText("Roll:  %.3f", drone_roll);
        ImGui::BulletText("Pitch: %.3f", drone_pitch);
        ImGui::BulletText("Yaw:   %.3f", drone_yaw);

        ImGui::End();
    }

    // Camera data window.
    ImGui::SetNextWindowSize(ImVec2(camera_window_width, camera_window_height), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(camera_window_xpos, camera_window_ypos), ImGuiCond_Always);
    {
        ImGui::Begin("Camera Data", NULL, imgui_window_flags);

        ImGui::Text("Camera Position");
        ImGui::BulletText("x: %.3f", camera_x);
        ImGui::BulletText("y: %.3f", camera_x);
        ImGui::BulletText("z: %.3f", camera_y);

        ImGui::Text("Target Position");
        ImGui::BulletText("x: %.3f", target_x);
        ImGui::BulletText("y: %.3f", target_y);
        ImGui::BulletText("z: %.3f", target_z);

        ImGui::End();
    }

    // Data queue window.
    ImGui::SetNextWindowSize(ImVec2(queue_window_width, queue_window_height), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(queue_window_xpos, queue_window_ypos), ImGuiCond_Always);
    {
        ImGui::Begin("Data Queue Elements", NULL, imgui_window_flags);

        ImGui::Text("Producer: %u", producer_n);
        ImGui::Text("Consumer: %u", consumer_n);

        ImGui::End();
    }
}

void ImgerManager::render()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImguiManager::update_drone_data(float dx, float dy, float dz,
    float roll, float pitch, float yaw)
{
    float drone_x = dx;
    float drone_y = dy;
    float drone_z = dz;
    float drone_roll = roll;
    float drone_pitch = pitch;
    float drone_yaw = yaw;
}

void ImguiManager::camera_data(float cx, float cy, float cz,
    float tx, float ty, float tz)
{
    camera_x = cx;
    camera_y = cy;
    camera_z = cz;
    target_x = tx;
    target_y = ty;
    target_z = tz;
}

void ImguiManager::update_queue_data(unsigned int p, unsigned int c)
{
    producer_n = p;
    consumer_n = c;
}

#endif /* IMGUI_MANAGER_HPP */
