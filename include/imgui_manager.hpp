#ifndef IMGUI_MANAGER_HPP
#define IMGUI_MANAGER_HPP

#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "com_port.hpp"
#include "resource_manager.hpp"
#include "shared.hpp"
#include "viewer_mode.hpp"

struct ImguiWindowSettings
{
    ImguiWindowSettings(float width_, float height_) :
        width(width_), height(height_) {}

    float width = 0.0;
    float height = 0.0;
    float xpos = 0.0;
    float ypos = 0.0;

    float bottom() const { return ypos + height; };

    void set_pos(float x, float y)
    {
        xpos = x;
        ypos = y;
    }
};

class ImguiManager
{
public:
    ImguiManager(GLFWwindow* window_,
                 const std::string& glsl_version,
                 std::size_t screen_width_,
                 std::size_t screen_height_,
                 ResourceManager* resource_manager_,
                 ViewerMode* viewer_mode_,
                 DroneData* drone_data_,
                 Camera* camera_,
                 bool show_demo_window_,
                 ComPort* com_port_);
    ~ImguiManager();

    bool init();

    void process_frame();
    void render();
    void render_draw_data();

    void update_screen_dimensions(std::size_t width, std::size_t height);
    void update_queue_data(unsigned int p, unsigned int c);
private:
    GLFWwindow* window;
    std::string glsl_version;

    std::size_t screen_width;
    std::size_t screen_height;

    const ImGuiWindowFlags imgui_window_flags = ImGuiWindowFlags_NoResize |
                                                ImGuiWindowFlags_NoCollapse;

    ImGuiIO io;

    ResourceManager* rm;

    ViewerMode* viewer_mode;

    bool show_demo_window;
    const ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    static constexpr float WINDOW_BUF = 20.0f;

    ImguiWindowSettings fps_win;
    ImguiWindowSettings mode_win;
    ImguiWindowSettings controls_t_win;
    ImguiWindowSettings controls_e_win;
    ImguiWindowSettings drone_win;
    ImguiWindowSettings camera_win;

    DroneData* drone_data;
    Camera* camera;

    ComPort* com_port;

    unsigned int producer_n = 0;
    unsigned int consumer_n = 0;

    void update_window_settings();
};

ImguiManager::ImguiManager(GLFWwindow* window_,
                           const std::string& glsl_version_,
                           std::size_t screen_width_,
                           std::size_t screen_height_,
                           ResourceManager* resource_manager_,
                           ViewerMode* viewer_mode_,
                           DroneData* drone_data_,
                           Camera* camera_,
                           bool show_demo_window_,
                           ComPort* com_port_) :
    window(window_),
    glsl_version(glsl_version_),
    fps_win(93.0, 32.0),
    mode_win(165.0, 80.0),
    controls_t_win(290.0, 130.0),
    controls_e_win(228.0, 82.0),
    drone_win(130.0, 167.0),
    camera_win(121.0, 167.0),
    rm(resource_manager_),
    viewer_mode(viewer_mode_),
    drone_data(drone_data_),
    camera(camera_),
    show_demo_window(show_demo_window_),
    com_port(com_port_)
{
    screen_width = screen_width_;
    screen_height = screen_height_;

    fps_win.set_pos(WINDOW_BUF, WINDOW_BUF);
    mode_win.set_pos(screen_width - WINDOW_BUF - mode_win.width, WINDOW_BUF);
    controls_t_win.set_pos(screen_width - WINDOW_BUF - controls_t_win.width,
        mode_win.bottom() + WINDOW_BUF);
    controls_e_win.set_pos(screen_width - WINDOW_BUF - controls_e_win.width,
        mode_win.bottom() + WINDOW_BUF);
    drone_win.set_pos(WINDOW_BUF, fps_win.bottom() + WINDOW_BUF);
    camera_win.set_pos(WINDOW_BUF, drone_win.bottom() + WINDOW_BUF);
}

bool ImguiManager::init()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    /*
     * Can't initialize ImGuiIO& in initializer list because it's necessary to
     * call ImGui::CreateContext() first. So instead, storing io as a solid type
     * rather than a reference, necessitating the use of this awkward
     * conversion. The original line is simply `io = ImGui::GetIO();`
     */
    io = std::remove_reference_t<decltype(ImGui::GetIO())>(ImGui::GetIO());

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version.c_str());

    return true;
}

ImguiManager::~ImguiManager()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImguiManager::process_frame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (show_demo_window)
    {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    // FPS window.
    ImGui::SetNextWindowSize(ImVec2(fps_win.width, fps_win.height), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(fps_win.xpos, fps_win.ypos), ImGuiCond_Always);
    {
        ImGui::Begin("FPS",
            NULL,
            imgui_window_flags | ImGuiWindowFlags_NoTitleBar);
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::End();
    }

    // Mode window.
    ImGui::SetNextWindowSize(ImVec2(mode_win.width, mode_win.height), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(mode_win.xpos, mode_win.ypos), ImGuiCond_Always);
    {
        ImGui::Begin("Application Mode", NULL, imgui_window_flags);

        static int e = 0;

        switch (*viewer_mode)
        {
        case ViewerMode::Telemetry:
            e = 0;
            break;
        case ViewerMode::Edit:
            e = 1;
            break;
        }

        if (ImGui::RadioButton("Telemetry (t)", &e, 0))
        {
            std::lock_guard<std::mutex> g(rm->viewer_mode_mutex);
            *viewer_mode = ViewerMode::Telemetry;
        }
        if (ImGui::RadioButton("Edit scene (e)", &e, 1))
        {
            std::lock_guard<std::mutex> g(rm->viewer_mode_mutex);
            *viewer_mode = ViewerMode::Edit;
        }

        ImGui::End();
    }

    // Controls window. Couldn't find good method to auto resize window, so it's
    // manual for now.
    {
        switch (*viewer_mode)
        {
        case ViewerMode::Telemetry:
        {
            ImGui::SetNextWindowSize(
                ImVec2(controls_t_win.width, controls_t_win.height),
                ImGuiCond_Always);
            ImGui::SetNextWindowPos(
                ImVec2(controls_t_win.xpos, controls_t_win.ypos),
                ImGuiCond_Always);
            ImGui::Begin("Telemetry Controls", NULL, imgui_window_flags);

            static int selected_port_idx = 0;
            std::vector<unsigned int> available_ports = com_port->get_available_ports();
            std::vector<const char*> port_list;
            for (auto& i : available_ports)
            {
                port_list.push_back(("COM" + std::to_string(i)).c_str());
            }

            ImGui::BulletText("Scan for COM ports (s)");
            ImGui::BulletText("Connect to COM port (c)");
            ImGui::BulletText("Start/Stop reading data (spacebar)");

            ImGui::Separator();

            ImGui::Text("Available COM ports:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(65);
            ImGui::Combo("",
                         &selected_port_idx,
                         port_list.data(),
                         port_list.size());

            ImGui::Text("Current COM port status: ");
            ImGui::SameLine();
            if (!com_port->is_connected())
            {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Disconnected");
            }
            else if (com_port->is_connected() && !com_port->is_reading())
            {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.1f, 1.0f), "COM%u ready", com_port->get_connected_port());
            }
            else if (com_port->is_connected() && com_port->is_reading())
            {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Reading COM%u", com_port->get_connected_port());
            }

            ImGui::End();
            break;
        }
        case ViewerMode::Edit:
        {
            ImGui::SetNextWindowSize(
                ImVec2(controls_e_win.width, controls_e_win.height),
                ImGuiCond_Always);
            ImGui::SetNextWindowPos(
                ImVec2(controls_e_win.xpos, controls_e_win.ypos),
                ImGuiCond_Always);
            ImGui::Begin("Edit Controls", NULL, imgui_window_flags);

            ImGui::BulletText("Camera control (1, default)");
            ImGui::BulletText("Drone control (2)");
            ImGui::BulletText("Reset view (r)");

            ImGui::End();
            break;
        }
        }
    }

    // Drone data window.
    ImGui::SetNextWindowSize(ImVec2(drone_win.width, drone_win.height),
        ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(drone_win.xpos, drone_win.ypos), ImGuiCond_Always);
    {
        ImGui::Begin("Drone Data", NULL, imgui_window_flags);

        ImGui::Text("Position");
        ImGui::BulletText("x:     %.3f", drone_data->position.x);
        ImGui::BulletText("y:     %.3f", drone_data->position.y);
        ImGui::BulletText("z:     %.3f", drone_data->position.z);

        ImGui::Text("Orientation");
        ImGui::BulletText("Roll:  %.3f", drone_data->orientation.x);
        ImGui::BulletText("Pitch: %.3f", drone_data->orientation.y);
        ImGui::BulletText("Yaw:   %.3f", drone_data->orientation.z);

        ImGui::End();
    }

    // Camera data window.
    ImGui::SetNextWindowSize(ImVec2(camera_win.width, camera_win.height),
        ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(camera_win.xpos, camera_win.ypos),
        ImGuiCond_Always);
    {
        ImGui::Begin("Camera Data", NULL, imgui_window_flags);

        ImGui::Text("Camera Position");
        ImGui::BulletText("x: %.3f", camera->get_position().x);
        ImGui::BulletText("y: %.3f", camera->get_position().y);
        ImGui::BulletText("z: %.3f", camera->get_position().z);

        ImGui::Text("Target Position");
        ImGui::BulletText("x: %.3f", camera->get_target().x);
        ImGui::BulletText("y: %.3f", camera->get_target().y);
        ImGui::BulletText("z: %.3f", camera->get_target().z);

        ImGui::End();
    }
}

void ImguiManager::render()
{
    ImGui::Render();
}

void ImguiManager::render_draw_data()
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImguiManager::update_screen_dimensions(std::size_t width,
    std::size_t height)
{
    screen_width = width;
    screen_height = height;

    update_window_settings();
}

void ImguiManager::update_window_settings()
{
    fps_win.set_pos(WINDOW_BUF, WINDOW_BUF);
    mode_win.set_pos(screen_width - mode_win.width - WINDOW_BUF, WINDOW_BUF);
    controls_t_win.set_pos(screen_width - controls_t_win.width,
        mode_win.bottom() + WINDOW_BUF);
    controls_e_win.set_pos(screen_width - controls_e_win.width,
        mode_win.bottom() + WINDOW_BUF);
    drone_win.set_pos(WINDOW_BUF, fps_win.bottom() + WINDOW_BUF);
    camera_win.set_pos(WINDOW_BUF, drone_win.bottom() + WINDOW_BUF);
}

void ImguiManager::update_queue_data(unsigned int p, unsigned int c)
{
    producer_n = p;
    consumer_n = c;
}

#endif /* IMGUI_MANAGER_HPP */
