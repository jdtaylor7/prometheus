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

#include "camera.hpp"
#include "printer.hpp"
#include "resource_manager.hpp"
#include "shader.hpp"
#include "vertex_data.hpp"

namespace fs = std::filesystem;

/*
 * Global data.
 */
constexpr std::size_t SCREEN_WIDTH = 1200;
constexpr std::size_t SCREEN_HEIGHT = 900;

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

        // glm::vec3 pos = camera.get_pos();
        // glm::vec3 target = camera.get_pos() + camera.get_front();
        //
        // std::cout << "pos.x = " << pos.x << '\n';
        // std::cout << "pos.y = " << pos.y << '\n';
        // std::cout << "pos.z = " << pos.z << '\n';
        //
        // std::cout << "target.x = " << target.x << '\n';
        // std::cout << "target.y = " << target.y << '\n';
        // std::cout << "target.z = " << target.z << '\n';
        // std::cout << '\n';
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        drone_pos.y += 0.003f;

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        drone_pos.y -= 0.003f;

    camera.update_pos(window);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    camera.update_angle(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.update_pov(yoffset);
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
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello Triangle", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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

    unsigned int fps_counter = 0;
    double fps_time = glfwGetTime();

    glm::mat4 room_model = glm::mat4(1.0f);
    room_model = glm::translate(room_model, room_pos);
    room_model = glm::scale(room_model, glm::vec3(room_size, room_size, room_size));

    /*
     * Render loop.
     */
    while (!glfwWindowShouldClose(window))
    {
        fps_counter++;
        if ((glfwGetTime() - fps_time) > 1.0)
        {
            fps_time = glfwGetTime();
            std::cout << "fps: " << fps_counter << '\n';
            fps_counter = 0;
        }

        camera.update_frames();

        /*
         * Input.
         */
        process_input(window);

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

        // Create transformation matrices.
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
        view = glm::lookAt(camera.get_pos(), camera.get_pos() + camera.get_front(), camera.get_up());
        projection = glm::perspective(glm::radians(camera.get_fov()), 800.0f / 600.0f, 0.1f, 100.0f);

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

        glBindVertexArray(0);

        /*
         * Swap buffers and poll I/O events.
         */
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    /*
     * Clean up.
     */
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}
