#ifndef OPENGL_MANAGER_HPP
#define OPENGL_MANAGER_HPP

#include <array>
#include <filesystem>
#include <iostream>
#include <memory>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include "shader.hpp"
#include "vertex_data.hpp"

namespace fs = std::filesystem;

class OpenglManager
{
public:
    OpenglManager(std::size_t screen_width_,
                  std::size_t screen_height_,
                  std::shared_ptr<DroneData> drone_data_,
                  std::shared_ptr<CameraData> camera_data_);
    ~OpenglManager();

    bool init();

    void process_frame();
private:
    std::size_t screen_width;
    std::size_t screen_height;

    const fs::path shader_dir = "src/shaders";
    const fs::path texture_dir = "include/textures";

    const fs::path vertex_shader_path = shader_dir / "shader.vs";
    const fs::path fragment_shader_path = shader_dir / "shader.fs";

    const fs::path container_texture_path = texture_dir / "container.jpg";
    const fs::path face_texture_path = texture_dir / "awesomeface.png";
    const fs::path wall_texture_path = texture_dir / "wall.jpg";

    static constexpr float room_size = 10.0f;
    static constexpr float drone_size = 0.2f;

    glm::vec3 room_pos{};
    std::shared_ptr<DroneData> drone_data;
    std::shared_ptr<CameraData> camera_data;

    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;

    Shader shader{};

    std::array<unsigned int, 2> textures{};
    std::size_t next_texture = 0;

    ResourceManager resource_manager{};
    std::unique_ptr<Camera> camera;

    glm::mat4 room_model;

    void make_opengl_objects();
    void configure_buffers();
    bool make_jpeg_texture(fs::path texture_path);
    bool make_png_texture(fs::path texture_path);
};

OpenglManager::OpenglManager(std::size_t screen_width_,
                             std::size_t screen_height,
                             std::shared_ptr<DroneData> drone_data_,
                             std::shared_ptr<CameraData> camera_data_) :
        screen_width(screen_width_),
        screen_height(screen_height),
        drone_data(drone_data_),
        camera_data(camera_data_)
{
    // Set dependent values.
    room_pos = glm::vec3(0.0, 0.0 + (room_size / 2), 0.0);

    // Create camera.
    camera = std::make_unique<Camera>(resource_manager,
        screen_width,
        screen_height,
        room_size / 2,
        room_size,
        camera_data);
}

bool OpenglManager::init()
{
    // Generate array and buffer objects.
    make_opengl_objects();
    configure_buffers();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Shader.
    shader = Shader(vertex_shader_path, fragment_shader_path);
    shader.configure();

    // Textures.
    glGenTextures(2, textures.data());
    stbi_set_flip_vertically_on_load(true);
    make_jpeg_texture(container_texture_path);
    make_jpeg_texture(wall_texture_path);

    // Set OpenGL flags.
    glEnable(GL_DEPTH_TEST);

    // Initialize room model.
    room_model = glm::mat4(1.0f);
    room_model = glm::translate(room_model, room_pos);
    room_model = glm::scale(room_model, glm::vec3(room_size, room_size, room_size));

    return true;
}

OpenglManager::~OpenglManager()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
}

void OpenglManager::process_frame()
{
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
    view = glm::lookAt(camera->get_pos(), camera->get_pos() + camera->get_front(), camera->get_up());
    projection = glm::perspective(glm::radians(camera->get_fov()), 800.0f / 600.0f, 0.1f, 100.0f);

    // Update uniforms.
    shader.use();
    shader.set_mat4fv("model", model);
    shader.set_mat4fv("view", view);
    shader.set_mat4fv("projection", projection);
    glBindVertexArray(vao);

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
    model = glm::translate(model, drone_data->position);
    model = glm::scale(model, glm::vec3(drone_size, drone_size, drone_size));
    shader.set_mat4fv("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Unbind VAO.
    glBindVertexArray(0);
}

bool OpenglManager::make_jpeg_texture(fs::path texture_path)
{
    // Activate a texture unit and bind the texture as the current GL_TEXTURE_2D.
    glBindTexture(GL_TEXTURE_2D, textures[next_texture]);
    next_texture++;

    // Set texture wrapping and filtering options.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load container texture.
    int width;
    int height;
    int num_channels;
    unsigned char *data = stbi_load(texture_path.c_str(), &width, &height, &num_channels, 0);

    // Generate texture.
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load texture at " << texture_path.string() << '\n';
        return false;
    }

    return true;
}

bool OpenglManager::make_png_texture(fs::path texture_path)
{
    // Activate a texture unit and bind the texture as the current GL_TEXTURE_2D.
    glBindTexture(GL_TEXTURE_2D, textures[next_texture]);
    next_texture++;

    // Set texture wrapping and filtering options.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load container texture.
    int width;
    int height;
    int num_channels;
    unsigned char *data = stbi_load(texture_path.c_str(), &width, &height, &num_channels, 0);

    // Generate texture.
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load texture at " << texture_path.string() << '\n';
        return false;
    }

    return true;
}

void OpenglManager::make_opengl_objects()
{
    // Generate and bind a "vertex array object" (VAO) to store the VBO and
    // corresponding vertex attribute configurations.
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Send vertex data to the vertex shader. Do so by allocating GPU
    // memory, which is managed by "vertex buffer objects" (VBOs).
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
}

void OpenglManager::configure_buffers()
{
    // Bind VBO to the vertex buffer object, GL_ARRAY_BUFFER. Buffer
    // operations on GL_ARRAY_BUFFER then apply to VBO.
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    // Bind EBO to the element buffer object, GL_ELEMENT_ARRAY_BUFFER. Buffer
    // operations on GL_ELEMENT_ARRAY_BUFFER then apply to EBO.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

    // Specify vertex data format.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

#endif /* OPENGL_MANAGER_HPP */
