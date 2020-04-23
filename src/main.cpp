#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "camera.hpp"
#include "fps_counter.hpp"
#include "printer.hpp"
#include "resource_manager.hpp"
#include "shader.hpp"
#include "vertex_data.hpp"

namespace fs = std::filesystem;

/*
 * Global data.
 */
constexpr std::size_t SCREEN_WIDTH = 1600;
constexpr std::size_t SCREEN_HEIGHT = 1200;
constexpr float WINDOW_BUF = 20.0f;

const fs::path shader_dir = "src/shaders";
const fs::path vertex_shader_path = shader_dir / "shader.vs";
const fs::path fragment_shader_path = shader_dir / "shader.fs";

const fs::path texture_dir = "include/textures";
const fs::path container_texture_path = texture_dir / "container.jpg";
const fs::path face_texture_path = texture_dir / "awesomeface.png";
const fs::path wall_texture_path = texture_dir / "wall.jpg";

constexpr float room_size = 10.0f;
constexpr float drone_size = 0.2f;

glm::vec3 room_pos(0.0, 0.0 + (room_size / 2), 0.0);
glm::vec3 drone_pos(0.0, 0.0f + (drone_size / 2), 0.0);

glm::vec3 initial_camera_pos(0.0, 1.0, 4.0);
glm::vec3 initial_camera_target(0.0, 1.0, 3.0);

bool display_fps = true;
float fps_update_rate_s = 1.0;

ImGuiWindowFlags imgui_window_flags = ImGuiWindowFlags_NoResize |
                                      ImGuiWindowFlags_NoCollapse;

// struct DroneData
// {
//     glm::vec3 pos;
// };
//
// DroneData drone_data(glm::vec3(0.0f, 0.0f, 0.0f));

/*
 * Global objects.
 */
ResourceManager resource_manager{};

Camera camera(resource_manager,
              SCREEN_WIDTH,
              SCREEN_HEIGHT,
              room_size / 2,
              room_size,
              initial_camera_pos,
              initial_camera_target);

Printer printer(resource_manager, camera);

/*
 * Callback functions.
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void process_input(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
    {
        printer.print_camera_details();
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        drone_pos.y += 0.003f;

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        drone_pos.y -= 0.003f;

    // camera.update_pos(window);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    // camera.update_angle(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // camera.update_pov(yoffset);
}

/*
 * Main function.
 */
int main()
{
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;

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
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Drone Viewer", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    /*
     * Load OpenGL function pointers.
     */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    /***************************************************************************
     * ImGUI start
     ***************************************************************************
     */
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    std::string glsl_version = "#version 330";
    ImGui_ImplOpenGL3_Init(glsl_version.c_str());

    bool show_demo_window = true;
    bool show_another_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    /***************************************************************************
     * ImGUI end
     ***************************************************************************
     */

    /*
     * Build buffers and vertex array object.
     */

    // Generate and bind a "vertex array object" (VAO) to store the VBO and
    // corresponding vertex attribute configurations.
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Send vertex data to the vertex shader. Do so by allocating GPU
    // memory, which is managed by "vertex buffer objects" (VBOs).
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind VBO to the vertex buffer object, GL_ARRAY_BUFFER. Buffer
    // operations on GL_ARRAY_BUFFER then apply to VBO.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    // Bind EBO to the element buffer object, GL_ELEMENT_ARRAY_BUFFER. Buffer
    // operations on GL_ELEMENT_ARRAY_BUFFER then apply to EBO.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

    // Specify vertex data format.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Can now unbind VAO and VBO, will rebind VAO as necessary in render loop.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    /*
     * Create shader program.
     */
    Shader shader(vertex_shader_path, fragment_shader_path);

    /*
     * Create textures.
     */

    // Create texture ID.
    std::vector<unsigned int> textures(2);
    glGenTextures(2, textures.data());

    // Activate a texture unit and bind the texture as the current GL_TEXTURE_2D.
    glBindTexture(GL_TEXTURE_2D, textures[0]);

    // Set texture wrapping and filtering options.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load container texture.
    int width;
    int height;
    int num_channels;
    unsigned char *data = stbi_load(container_texture_path.c_str(), &width, &height, &num_channels, 0);

    // Generate texture.
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load texture\n";
    }

    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load and generate face texture.
    stbi_set_flip_vertically_on_load(true);
    data = stbi_load(wall_texture_path.c_str(), &width, &height, &num_channels, 0);
    if (data)
    {
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load texture\n";
    }

    glEnable(GL_DEPTH_TEST);

    // Initialize room model.
    glm::mat4 room_model = glm::mat4(1.0f);
    room_model = glm::translate(room_model, room_pos);
    room_model = glm::scale(room_model, glm::vec3(room_size, room_size, room_size));

    // Initialize fps counter.
    FpsCounter fps(display_fps, fps_update_rate_s);

    /*
     * Render loop.
     */
    while (!glfwWindowShouldClose(window))
    {
        /***********************************************************************
         * ImGUI start
         ***********************************************************************
         */
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // {
        //     ImGui::ShowDemoWindow(&show_demo_window);
        // }

        // FPS window.
        float fps_window_width = 93.0;
        float fps_window_height = 32.0;
        float fps_window_xpos = WINDOW_BUF;
        float fps_window_ypos = WINDOW_BUF;
        ImGui::SetNextWindowSize(ImVec2(fps_window_width, fps_window_height), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(fps_window_xpos, fps_window_ypos), ImGuiCond_Always);
        {
            ImGui::Begin("FPS", NULL, imgui_window_flags | ImGuiWindowFlags_NoTitleBar);
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // Mode window.
        float mode_window_width = 165.0;
        float mode_window_height = 123.0;
        float mode_window_xpos = SCREEN_WIDTH - mode_window_width - WINDOW_BUF;
        float mode_window_ypos = WINDOW_BUF;
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
        float controls_window_width = 163.0;
        float controls_window_height = 82.0;
        float controls_window_xpos = SCREEN_WIDTH - controls_window_width - WINDOW_BUF;
        float controls_window_ypos = mode_window_ypos + mode_window_height + WINDOW_BUF;
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
        float drone_window_width = 121.0;
        float drone_window_height = 167.0;
        float drone_window_xpos = WINDOW_BUF;
        float drone_window_ypos = fps_window_ypos + fps_window_height + WINDOW_BUF;
        ImGui::SetNextWindowSize(ImVec2(drone_window_width, drone_window_height), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(drone_window_xpos, drone_window_ypos), ImGuiCond_Always);
        {
            ImGui::Begin("Drone Data", NULL, imgui_window_flags);

            ImGui::Text("Position");
            ImGui::BulletText("x:     %.3f", 0.000f);
            ImGui::BulletText("y:     %.3f", 0.000f);
            ImGui::BulletText("z:     %.3f", 0.000f);

            ImGui::Text("Angle");
            ImGui::BulletText("Roll:  %.3f", 0.000f);
            ImGui::BulletText("Pitch: %.3f", 0.000f);
            ImGui::BulletText("Yaw:   %.3f", 0.000f);

            ImGui::End();
        }

        // Camera data window.
        float camera_window_width = 121.0;
        float camera_window_height = 167.0;
        float camera_window_xpos = WINDOW_BUF;
        float camera_window_ypos = drone_window_ypos + drone_window_height + WINDOW_BUF;
        ImGui::SetNextWindowSize(ImVec2(camera_window_width, camera_window_height), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(camera_window_xpos, camera_window_ypos), ImGuiCond_Always);
        {
            ImGui::Begin("Camera Data", NULL, imgui_window_flags);

            ImGui::Text("Camera Position");
            ImGui::BulletText("x: %.3f", 0.000f);
            ImGui::BulletText("y: %.3f", 0.000f);
            ImGui::BulletText("z: %.3f", 0.000f);

            ImGui::Text("Target Position");
            ImGui::BulletText("x: %.3f", 0.000f);
            ImGui::BulletText("y: %.3f", 0.000f);
            ImGui::BulletText("z: %.3f", 0.000f);

            ImGui::End();
        }

        // Data queue window.
        float queue_window_width = 171.0;
        float queue_window_height = 65.0;
        float queue_window_xpos = WINDOW_BUF;
        float queue_window_ypos = camera_window_ypos + camera_window_height + WINDOW_BUF;
        ImGui::SetNextWindowSize(ImVec2(queue_window_width, queue_window_height), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(queue_window_xpos, queue_window_ypos), ImGuiCond_Always);
        {
            ImGui::Begin("Data Queue Elements", NULL, imgui_window_flags);

            ImGui::Text("Producer: %u", 0);
            ImGui::Text("Consumer: %u", 0);

            ImGui::End();
        }

        ImGui::Render();

        /***********************************************************************
         * ImGUI end
         ***********************************************************************
         */

        // Compute fps.
        fps.update();

        /*
         * Process input.
         */
        process_input(window);

        /*
         * Update camera.
         */
        camera.update_frames();

        /*
         * Render.
         */

        // Color buffer.
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render square.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textures[1]);

        // Update transformation matrices.
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);
        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
        view = glm::lookAt(camera.get_pos(), camera.get_pos() + camera.get_front(), camera.get_up());
        projection = glm::perspective(glm::radians(camera.get_fov()), 800.0f / 600.0f, 0.1f, 100.0f);

        // Update uniforms.
        shader.use();
        shader.set_mat4fv("model", model);
        shader.set_mat4fv("view", view);
        shader.set_mat4fv("projection", projection);
        glBindVertexArray(VAO);

        // Draw room.
        shader.set_bool("use_texture", true);
        shader.set_int("texture1", 0);
        shader.set_int("texture2", 0);
        shader.set_mat4fv("model", room_model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Draw drone.
        shader.set_bool("use_texture", true);
        shader.set_int("texture1", 1);
        shader.set_int("texture2", 1);
        model = glm::mat4(1.0f);
        model = glm::translate(model, drone_pos);
        model = glm::scale(model, glm::vec3(drone_size, drone_size, drone_size));
        shader.set_mat4fv("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Unbind VAO.
        glBindVertexArray(0);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        /*
         * Swap buffers and poll I/O events.
         */
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    /*
     * Clean up.
     */
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}
