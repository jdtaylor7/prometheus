#ifndef IMGUI_MANAGER_HPP
#define IMGUI_MANAGER_HPP

#include <string>
#include <type_traits>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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
    ImguiManager(GLFWwindow* window_, const std::string& glsl_version,
        std::size_t screen_width_, std::size_t screen_height_);
    ~ImguiManager();

    void execute_frame();
    void render();
    void render_draw_data();

    void update_screen_dimensions(std::size_t width, std::size_t height);
    void update_drone_data(float dx, float dy, float dz,
        float roll, float pitch, float yaw);
    void camera_data(float cx, float cy, float cz,
        float tx, float ty, float tz);
    void update_queue_data(unsigned int p, unsigned int c);
private:
    GLFWwindow* window;
    std::size_t screen_width;
    std::size_t screen_height;

    const ImGuiWindowFlags imgui_window_flags = ImGuiWindowFlags_NoResize |
                                                ImGuiWindowFlags_NoCollapse;

    ImGuiIO io;

    bool show_demo_window = false;
    const ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    const float WINDOW_BUF = 20.0f;

    ImguiWindowSettings fps;
    ImguiWindowSettings mode;
    ImguiWindowSettings controls;
    ImguiWindowSettings drone;
    ImguiWindowSettings camera;
    ImguiWindowSettings queue;

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

    void update_window_settings();
};

ImguiManager::ImguiManager(GLFWwindow* window_,
                           const std::string& glsl_version,
                           std::size_t screen_width_,
                           std::size_t screen_height_) :
    window(window_),
    fps(93.0, 32.0),
    mode(165.0, 80.0),
    controls(163.0, 82.0),
    drone(121.0, 167.0),
    camera(121.0, 167.0),
    queue(171.0, 65.0)
{
    screen_width = screen_width_;
    screen_height = screen_height_;

    fps.set_pos(WINDOW_BUF, WINDOW_BUF);
    mode.set_pos(screen_width - mode.width - WINDOW_BUF, WINDOW_BUF);
    controls.set_pos(mode.xpos, mode.bottom() + WINDOW_BUF);
    drone.set_pos(WINDOW_BUF, fps.bottom() + WINDOW_BUF);
    camera.set_pos(WINDOW_BUF, drone.bottom() + WINDOW_BUF);
    queue.set_pos(WINDOW_BUF, camera.bottom() + WINDOW_BUF);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    /*
     * Can't initialize ImGuiIO& in initializer list because it's necessary to
     * call ImGui::CreateContext() first. So instead, storing io as a solid type
     * rather than a reference, necessitating the use of this awkward
     * conversion.
     */
    // io = ImGui::GetIO();
    io = std::remove_reference_t<decltype(ImGui::GetIO())>(ImGui::GetIO());

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version.c_str());
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
    ImGui_ImplGlfw_NewFrame(); // offending line
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
    ImGui::SetNextWindowSize(ImVec2(mode.width, mode.height), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(mode.xpos, mode.ypos), ImGuiCond_Always);
    {
        ImGui::Begin("Application Mode", NULL, imgui_window_flags);

        static int e = 0;
        ImGui::RadioButton("Telemetry (t)", &e, 0);
        ImGui::RadioButton("Edit scene (e)", &e, 1);

        ImGui::End();
    }

    // Controls window.
    ImGui::SetNextWindowSize(ImVec2(controls.width, controls.height), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(controls.xpos, controls.ypos), ImGuiCond_Always);
    {
        ImGui::Begin("Simulation Controls", NULL, imgui_window_flags);

        ImGui::BulletText("Start/Stop (space)");
        ImGui::BulletText("Pause (p)");
        ImGui::BulletText("Reset (r)");

        ImGui::End();
    }

    // Drone data window.
    ImGui::SetNextWindowSize(ImVec2(drone.width, drone.height), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(drone.xpos, drone.ypos), ImGuiCond_Always);
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
    ImGui::SetNextWindowSize(ImVec2(camera.width, camera.height), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(camera.xpos, camera.ypos), ImGuiCond_Always);
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
    ImGui::SetNextWindowSize(ImVec2(queue.width, queue.height), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(queue.xpos, queue.ypos), ImGuiCond_Always);
    {
        ImGui::Begin("Data Queue Elements", NULL, imgui_window_flags);

        ImGui::Text("Producer: %u", producer_n);
        ImGui::Text("Consumer: %u", consumer_n);

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
    fps.set_pos(WINDOW_BUF, WINDOW_BUF);
    mode.set_pos(screen_width - mode.width - WINDOW_BUF, WINDOW_BUF);
    controls.set_pos(screen_width - controls.width, mode.bottom() + WINDOW_BUF);
    drone.set_pos(WINDOW_BUF, fps.bottom() + WINDOW_BUF);
    camera.set_pos(WINDOW_BUF, drone.bottom() + WINDOW_BUF);
    queue.set_pos(WINDOW_BUF, camera.bottom() + WINDOW_BUF);
}

void ImguiManager::update_drone_data(float dx, float dy, float dz,
    float roll, float pitch, float yaw)
{
    drone_x = dx;
    drone_y = dy;
    drone_z = dz;
    drone_roll = roll;
    drone_pitch = pitch;
    drone_yaw = yaw;
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
