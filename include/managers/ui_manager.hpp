#ifndef UI_MANAGER_HPP
#define UI_MANAGER_HPP

#include <memory>
#include <mutex>
#include <string>
#include <type_traits>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

#include "resource_manager.hpp"
#include "serial_port.hpp"
#include "shared.hpp"
#include "viewer_mode.hpp"

struct ScrollingData {
    int MaxSize = 1000;
    int Offset  = 0;
    ImVector<ImVec2> Data;
    ScrollingData() { Data.reserve(MaxSize); }
    void AddPoint(float x, float y) {
        if (Data.size() < MaxSize)
            Data.push_back(ImVec2(x,y));
        else {
            Data[Offset] = ImVec2(x,y);
            Offset =  (Offset + 1) % MaxSize;
        }
    }
    void Erase() {
        if (Data.size() > 0) {
            Data.shrink(0);
            Offset  = 0;
        }
    }
};

struct UiWindowSettings
{
    UiWindowSettings(float width_, float height_) :
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

class UiManager
{
public:
    UiManager(GLFWwindow* window_,
                 const std::string& glsl_version,
                 std::size_t screen_width_,
                 std::size_t screen_height_,
                 ResourceManager* resource_manager_,
                 ViewerMode* viewer_mode_,
                 DroneData* drone_data_,
                 Camera* camera_,
                 SerialPort* serial_port_,
                 bool show_demo_window_,
                 bool show_implot_demo_window_,
                 bool show_camera_data_window_);
    ~UiManager();

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
    bool show_implot_demo_window;
    bool show_camera_data_window;
    const ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    static constexpr float WINDOW_BUF = 20.0f;

    UiWindowSettings fps_win;
    UiWindowSettings mode_win;
    UiWindowSettings controls_t_win;
    UiWindowSettings controls_e_win;
    UiWindowSettings drone_win;
    UiWindowSettings camera_win;

    DroneData* drone_data;
    Camera* camera;

    SerialPort* serial_port;

    unsigned int producer_n = 0;
    unsigned int consumer_n = 0;

    void update_window_settings();
};

UiManager::UiManager(GLFWwindow* window_,
                           const std::string& glsl_version_,
                           std::size_t screen_width_,
                           std::size_t screen_height_,
                           ResourceManager* resource_manager_,
                           ViewerMode* viewer_mode_,
                           DroneData* drone_data_,
                           Camera* camera_,
                           SerialPort* serial_port_,
                           bool show_demo_window_,
                           bool show_implot_demo_window_,
                           bool show_camera_data_window_) :
    window(window_),
    glsl_version(glsl_version_),
    fps_win(93.0, 32.0),
    mode_win(165.0, 80.0),
#ifdef OS_CYGWIN
    controls_t_win(310.0, 130.0),
#elif OS_LINUX
    controls_t_win(275.0, 165.0),
#endif
    controls_e_win(290.0, 170.0),
    drone_win(300.0, 480.0),
    camera_win(150.0, 220.0),
    rm(resource_manager_),
    viewer_mode(viewer_mode_),
    drone_data(drone_data_),
    camera(camera_),
    serial_port(serial_port_),
    show_implot_demo_window(show_implot_demo_window_),
    show_demo_window(show_demo_window_),
    show_camera_data_window(show_camera_data_window_)
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

bool UiManager::init()
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

UiManager::~UiManager()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UiManager::process_frame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (show_demo_window)
    {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    if (show_implot_demo_window)
    {
        ImGui::ShowImPlotDemoWindow(&show_implot_demo_window);
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
            std::vector<std::string> available_ports = serial_port->get_available_ports();
            std::vector<const char*> port_list;
            for (auto& s : available_ports)
            {
                port_list.push_back(s.c_str());
            }

            ImGui::BulletText("Scan for serial devices (s)");
            ImGui::BulletText("Connect to serial devices (c)");
            ImGui::BulletText("Start/Stop reading data (spacebar)");

            ImGui::Separator();

            ImGui::Text("Available serial devices:");
#ifdef OS_CYGWIN
            ImGui::SetNextItemWidth(65);
#elif OS_LINUX
            ImGui::SetNextItemWidth(115);
#endif
            ImGui::Combo("",
                         &selected_port_idx,
                         port_list.data(),
                         port_list.size());

            ImGui::Text("Current serial port status:");
            if (!serial_port->is_open())
            {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Disconnected");
            }
            else if (serial_port->is_open() && !serial_port->is_reading())
            {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.1f, 1.0f), "%s ready", serial_port->get_port_name().c_str());
            }
            else if (serial_port->is_open() && serial_port->is_reading())
            {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Reading %s", serial_port->get_port_name().c_str());
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

            ImGui::BulletText("Camera control");
            ImGui::Indent();
            ImGui::BulletText("Look around: Mouse");
            ImGui::BulletText("Horizontal controls: WASD");
            ImGui::BulletText("Vertical controls: Space/Control");
            ImGui::BulletText("Increase speed: Hold shift");
            ImGui::Unindent();
            ImGui::BulletText("Drone control");
            ImGui::Indent();
            ImGui::BulletText("Vertical controls: Up/Down arrows");
            ImGui::Unindent();
            ImGui::BulletText("Reset scene (r)");

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

        static ScrollingData sdata1;
        static ScrollingData sdata2;
        static ScrollingData sdata3;
        static float t = 0;
        t += ImGui::GetIO().DeltaTime;
        sdata1.AddPoint(t, drone_data->position.x * 0.3f);
        sdata2.AddPoint(t, drone_data->position.y * 0.3f);
        sdata3.AddPoint(t, drone_data->position.z * 0.3f);
        ImGui::SetNextPlotRangeX(t - 10, t, ImGuiCond_Always);
        static int rt_axis = ImAxisFlags_Default & ~ImAxisFlags_TickLabels;
        if (ImGui::BeginPlot("##Drone Position Data", NULL, NULL, {-1, 150}, ImPlotFlags_Default, rt_axis, rt_axis))
        {
            ImGui::Plot("X Position", &sdata1.Data[0].x, &sdata1.Data[0].y, sdata1.Data.size(), sdata1.Offset, 2 * sizeof(float));
            ImGui::Plot("Y Position", &sdata2.Data[0].x, &sdata2.Data[0].y, sdata2.Data.size(), sdata2.Offset, 2 * sizeof(float));
            ImGui::Plot("Z Position", &sdata3.Data[0].x, &sdata3.Data[0].y, sdata3.Data.size(), sdata3.Offset, 2 * sizeof(float));
            ImGui::EndPlot();
        }

        ImGui::Text("Orientation");
        ImGui::BulletText("Roll:  %.3f", drone_data->orientation.x);
        ImGui::BulletText("Pitch: %.3f", drone_data->orientation.y);
        ImGui::BulletText("Yaw:   %.3f", drone_data->orientation.z);

        static ScrollingData sdata4;
        static ScrollingData sdata5;
        static ScrollingData sdata6;
        sdata4.AddPoint(t, drone_data->orientation.x * 0.5f);
        sdata5.AddPoint(t, drone_data->orientation.y * 0.5f);
        sdata6.AddPoint(t, drone_data->orientation.z * 0.5f);
        ImGui::SetNextPlotRangeX(t - 10, t, ImGuiCond_Always);
        if (ImGui::BeginPlot("##Drone Orientation Data", NULL, NULL, {-1, 150}, ImPlotFlags_Default, rt_axis, rt_axis))
        {
            ImGui::Plot("X Orientation", &sdata4.Data[0].x, &sdata4.Data[0].y, sdata4.Data.size(), sdata4.Offset, 2 * sizeof(float));
            ImGui::Plot("Y Orientation", &sdata5.Data[0].x, &sdata5.Data[0].y, sdata5.Data.size(), sdata5.Offset, 2 * sizeof(float));
            ImGui::Plot("Z Orientation", &sdata6.Data[0].x, &sdata6.Data[0].y, sdata6.Data.size(), sdata6.Offset, 2 * sizeof(float));
            ImGui::EndPlot();
        }

        ImGui::End();
    }

    // Camera data window.
    if (show_camera_data_window)
    {
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

            ImGui::Text("Camera Front");
            ImGui::BulletText("x: %.3f", camera->get_front().x);
            ImGui::BulletText("y: %.3f", camera->get_front().y);
            ImGui::BulletText("z: %.3f", camera->get_front().z);

            ImGui::Text("Camera Orientation");
            ImGui::BulletText("pitch: %.3f", camera->get_pitch());
            ImGui::BulletText("yaw: %.3f", camera->get_yaw());

            ImGui::End();
        }
    }
}

void UiManager::render()
{
    ImGui::Render();
}

void UiManager::render_draw_data()
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UiManager::update_screen_dimensions(std::size_t width,
    std::size_t height)
{
    screen_width = width;
    screen_height = height;

    update_window_settings();
}

void UiManager::update_window_settings()
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

void UiManager::update_queue_data(unsigned int p, unsigned int c)
{
    producer_n = p;
    consumer_n = c;
}

#endif /* UI_MANAGER_HPP */
