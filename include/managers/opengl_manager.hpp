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
#include "quad.hpp"
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

    void pass_objects(SceneLighting*, Room*, Model*, Quad*);
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
    const fs::path shadow_vshader_path = shader_path / "shadow.vs";
    const fs::path shadow_fshader_path = shader_path / "shadow.fs";
    const fs::path quad_vshader_path = shader_path / "quad.vs";
    const fs::path quad_fshader_path = shader_path / "quad.fs";

    float fov = 45.0;
    float drone_scale_factor = 0.002f;

    glm::vec3 room_dimensions;

    /*
     * Shadow settings.
     */
    const std::size_t shadow_width = 4096;
    const std::size_t shadow_height = 4096;
    unsigned int depth_map;
    unsigned int depth_map_fbo;
    bool generate_shadows = true;

    // Light frustum settings.
    float light_frustum_near_plane = 0.1f;
    float light_frustum_far_plane = 30.0f;
    float light_fov = 90.0f;
    glm::mat4 light_projection = glm::perspective(
        glm::radians(light_fov),
        float(shadow_width / shadow_height),
        light_frustum_near_plane,
        light_frustum_far_plane);

    // glm::vec3 room_pos{};
    DroneData* drone_data;
    Camera* camera;
    ResourceManager* resource_manager;

    /*
     * Shaders.
     */
    std::unique_ptr<Shader> plight_shader;
    std::unique_ptr<Shader> main_shader;
    std::unique_ptr<Shader> shadow_shader;
    std::unique_ptr<Shader> quad_shader;

    /*
     * Models.
     */
    SceneLighting* sl;
    Room* room;
    Model* drone;
    Quad* quad;
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
    shadow_shader = std::make_unique<Shader>(shadow_vshader_path, shadow_fshader_path);
    shadow_shader->init();
    quad_shader = std::make_unique<Shader>(quad_vshader_path, quad_fshader_path);
    quad_shader->init();

    /*
     * Set up shadow mapping.
     */
    // Create framebuffer for depth map.
    std::cout << "Init shadow map\n";
    glGenFramebuffers(1, &depth_map_fbo);

    // Create texture for depth map.
    glGenTextures(1, &depth_map);
    glBindTexture(GL_TEXTURE_2D, depth_map);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadow_width,
        shadow_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    std::vector<float> border_color = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    // Attach depth texture as framebuffer's depth buffer.
    glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "OpenglManager::init: Framebuffer incomplete\n";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Set shader attributes.
    main_shader->use();
    main_shader->set_int("material.texture_diffuse1", 0);
    main_shader->set_int("material.texture_specular1", 1);
    main_shader->set_int("shadow_map", 2);
    main_shader->set_float("material.shininess", 32.0f);

    return true;
}

OpenglManager::~OpenglManager()
{
    // glDeleteVertexArrays(1, &vao);
    // glDeleteBuffers(1, &vbo);
    // glDeleteBuffers(1, &ebo);
}

void OpenglManager::pass_objects(SceneLighting* sl_, Room* room_, Model* model_, Quad* quad_)
{
    sl = sl_;
    room = room_;
    drone = model_;
    quad = quad_;
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
     * Generate depth buffer for shadows.
     */
    // Set up light perspective matrix. This part is a bit of a hack since
    // we're pretending a point light is a directional light (by using a
    // lookAt matrix which always looks at the model). This is fine since we
    // only have one model in the scene. It also means we can use shadow
    // mapping instead of point shadows, which is simpler.
    if (!sl->points[0])
        generate_shadows = false;

    // std::cout << "generate_shadows = " << generate_shadows << '\n';
    glm::mat4 light_space_matrix;
    if (generate_shadows)
    {
        glm::mat4 light_view = glm::lookAt(
            sl->points[0]->position, drone_data->position, glm::vec3(0.0f, 1.0f, 0.0f));
        light_space_matrix = light_projection * light_view;

        // Pass uniforms to shader.
        shadow_shader->use();
        shadow_shader->set_mat4fv("light_space_matrix", light_space_matrix);

        glViewport(0, 0, shadow_width, shadow_height);
        glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Render scene to shadow map. Cull front faces during to eliminate
        // potential peter panning.
        // glCullFace(GL_FRONT);
        render_scene(shadow_shader.get());
        // glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    else
    {
        std::cout << "OpenglManager::process_frame: Not generating shadows.\n";
    }

    /*
     * Render.
     */
    main_shader->use();
    main_shader->set_bool("smooth_shadows", false);

    // Reset viewport.
    glViewport(0, 0, screen_width, screen_height);

    // Reset buffers.
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

    if (generate_shadows)
    {
        // Pass light space matrix to main main_shader.
        main_shader->set_mat4fv("light_space_matrix", light_space_matrix);

        // Pass depth map to objects, to render shadows.
        room->set_depth_map(depth_map);
        drone->set_depth_map(depth_map);
    }

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

    // // Render quad. TODO for testing only.
    // quad_shader->use();
    // quad_shader->set_float("near_plane", light_frustum_near_plane);
    // quad_shader->set_float("far_plane", light_frustum_far_plane);
    // // quad_shader->set_int("depth_map", 0);
    // quad->set_depth_map(depth_map);
    // quad->draw(quad_shader.get());
}

#endif /* OPENGL_MANAGER_HPP */
