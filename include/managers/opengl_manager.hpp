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

#include "lights.hpp"
#include "model.hpp"
#include "room.hpp"
#include "shader.hpp"
#include "vertex_data.hpp"
#include "utility.hpp"

namespace fs = std::filesystem;

class OpenglManager
{
public:
    OpenglManager(std::size_t screen_width_,
        std::size_t screen_height_,
        glm::vec3 room_dimensions_,
        ResourceManager* resource_manager_,
        DroneData* drone_data_,
        Camera* camera_) :
            screen_width(screen_width_),
            screen_height(screen_height_),
            room_dimensions(room_dimensions_),
            resource_manager(resource_manager_),
            drone_data(drone_data_),
            camera(camera_)
    {
    }
    ~OpenglManager();

    bool init();

    void pass_objects(SceneLighting*, Room*, Model*);
    void render_scene(Shader*);

    void process_frame();
private:
    std::size_t screen_width;
    std::size_t screen_height;

    /*
     * Shader paths.
     */
    const fs::path shader_path = "src/shaders";

    const fs::path main_vshader_path = shader_path / "main.vs";
    const fs::path main_fshader_path = shader_path / "main.fs";
    const fs::path plight_vshader_path = shader_path / "point_light.vs";
    const fs::path plight_fshader_path = shader_path / "point_light.fs";

    float fov = 45.0;
    float drone_scale_factor = 0.002f;

    glm::vec3 room_dimensions;

    // glm::vec3 room_pos{};
    DroneData* drone_data;
    Camera* camera;
    ResourceManager* resource_manager;

    /*
     * Shaders.
     */
    std::unique_ptr<Shader> plight_shader;
    std::unique_ptr<Shader> main_shader;

    /*
     * Models.
     */
    SceneLighting* sl;
    Room* room;
    Model* drone;
};

bool OpenglManager::init()
{
    /*
     * Set global OpenGL state.
     */
    glEnable(GL_DEPTH_TEST);

    /*
     * Create shaders.
     */
    plight_shader = std::make_unique<Shader>(plight_vshader_path, plight_fshader_path);
    plight_shader->init();
    main_shader = std::make_unique<Shader>(main_vshader_path, main_fshader_path);
    main_shader->init();

    /*
     * Set up shadow mapping.
     */

    return true;
}

OpenglManager::~OpenglManager()
{
    // glDeleteVertexArrays(1, &vao);
    // glDeleteBuffers(1, &vbo);
    // glDeleteBuffers(1, &ebo);
}

void OpenglManager::pass_objects(SceneLighting* sl_, Room* room_, Model* model_)
{
    sl = sl_;
    room = room_;
    drone = model_;
}

void OpenglManager::render_scene(Shader* shader)
{
    if (!shader)
    {
        std::cerr << "opengl_manager::render_scene: shader is NULL\n";
        return;
    }
    /*
     * Draw room.
     */
    shader->use();

    // Position properties.
    shader->set_vec3("view_pos", camera->get_position());

    // Material properties.
    shader->set_float("material.shininess", 32.0f);

    // Render room.
    if (!room)
    {
        std::cerr << "opengl_manager::render_scene: room is NULL\n";
        return;
    }
    room->draw(shader);

    /*
     * Draw model.
     */
    shader->use();

    // Position properties.
    shader->set_vec3("view_pos", camera->get_position());

    // Set model matrix.
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, drone_data->position);
    model = glm::scale(model, glm::vec3(drone_scale_factor));
    shader->set_mat4fv("model", model);

    // Render drone.
    if (!drone)
    {
        std::cerr << "opengl_manager::render_scene: drone is NULL\n";
        return;
    }
    drone->draw(shader);
}

void OpenglManager::process_frame()
{
    // Color buffer.
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /*
     * Render.
     */
    main_shader->use();

    // Reset viewport.
    glViewport(0, 0, screen_width, screen_height);

    // Initial projection and view matrix definitions.
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    // Set view and projection matrices. Model matrix set per object in
    // render_scene function.
    view = glm::lookAt(camera->get_position(), camera->get_position() + camera->get_front(), camera->get_up());
    projection = glm::perspective(glm::radians(fov), float(screen_width / screen_height), 0.1f, 100.0f);

    // Assign projection and view matrices.
    main_shader->set_mat4fv("projection", projection);
    main_shader->set_mat4fv("view", view);

    // // Pass light space matrix to main main_shader.
    // main_shader->set_mat4fv("light_space_matrix", light_space_matrix);

    // // Pass depth map to objects, to render shadows.
    // room->set_depth_map(depth_map);
    // drone->set_depth_map(depth_map);

    // Render scene normally.
    render_scene(main_shader.get());

    /*
     * Draw point lights.
     */
    plight_shader->use();

    // Set MVP matrices.
    plight_shader->set_mat4fv("projection", projection);
    plight_shader->set_mat4fv("view", view);

    // Render point light(s).
    for (auto& point_light : sl->points)
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model, point_light->position);
        model = glm::scale(model, glm::vec3(point_light->scale_factor));
        plight_shader->set_mat4fv("model", model);
        plight_shader->set_vec3("color", point_light->color);
        point_light->draw();
    }

    // // Render square.
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, textures[0]);
    // glActiveTexture(GL_TEXTURE1);
    // glBindTexture(GL_TEXTURE_2D, textures[1]);
    //
    // // Update transformation matrices.
    // glm::mat4 model = glm::mat4(1.0f);
    // glm::mat4 view = glm::mat4(1.0f);
    // glm::mat4 projection = glm::mat4(1.0f);
    // model = glm::rotate(model,
    //     (float)glfwGetTime() * glm::radians(50.0f),
    //     glm::vec3(0.5f, 1.0f, 0.0f));
    // view = glm::lookAt(camera->get_position(),
    //     camera->get_position() + camera->get_front(),
    //     camera->get_up());
    // projection = glm::perspective(glm::radians(camera->get_fov()),
    //     800.0f / 600.0f,
    //     0.1f,
    //     100.0f);
    //
    // // Update uniforms.
    // shader.use();
    // shader.set_mat4fv("model", model);
    // shader.set_mat4fv("view", view);
    // shader.set_mat4fv("projection", projection);
    // glBindVertexArray(vao);
    //
    // // Draw room.
    // shader.set_bool("use_texture", true);
    // shader.set_int("texture1", 0);
    // shader.set_int("texture2", 0);
    // shader.set_mat4fv("model", room_model);
    // glDrawArrays(GL_TRIANGLES, 0, 36);
    //
    // // Draw drone.
    // shader.set_bool("use_texture", true);
    // shader.set_int("texture1", 1);
    // shader.set_int("texture2", 1);
    // model = glm::mat4(1.0f);
    // model = glm::translate(model, drone_data->position);
    // model = glm::scale(model, glm::vec3(drone_data->size, drone_data->size, drone_data->size));
    // shader.set_mat4fv("model", model);
    // glDrawArrays(GL_TRIANGLES, 0, 36);
    //
    // // Unbind VAO.
    // glBindVertexArray(0);
}

#endif /* OPENGL_MANAGER_HPP */
