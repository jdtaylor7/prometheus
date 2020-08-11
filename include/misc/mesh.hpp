#ifndef MESH_HPP
#define MESH_HPP

#include <filesystem>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "lights.hpp"
#include "shader.hpp"

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coords;
};

struct Texture
{
    std::size_t id;
    std::string type;
    std::filesystem::path path;
};

class Mesh
{
public:
    Mesh(std::vector<Vertex> vertices_,
        std::vector<unsigned int> indices_,
        std::vector<Texture> textures_,
        SceneLighting* scene_lighting_) :
            vertices(vertices_),
            indices(indices_),
            textures(textures_),
            sl(scene_lighting_)
    {
    }

    void init();
    void deinit();
    void draw(Shader* shader);

    void set_depth_map(unsigned int);
private:
    SceneLighting* sl;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;

    unsigned int depth_map;
    bool depth_map_set = false;
};

void Mesh::init()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

    // Vertex positions.
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // Vertex normals.
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // Vertex textures coordinates.
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));

    glBindVertexArray(0);
}

void Mesh::deinit()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
}

void Mesh::draw(Shader* shader)
{
    // Set shader attributes.
    if (!shader)
    {
        std::cerr << "Room::draw: SceneLighting->Spotlight pointer is null.\n";
        return;
    }
    shader->use();

    // Directional light properties.
    if (!sl->dir)
    {
        std::cerr << "Room::draw: SceneLighting->DirectionalLight pointer is null.\n";
        return;
    }

    shader->set_vec3("dir_light.direction", sl->dir->direction);

    shader->set_vec3("dir_light.ambient", sl->dir->ambient);
    shader->set_vec3("dir_light.diffuse", sl->dir->diffuse);
    shader->set_vec3("dir_light.specular", sl->dir->specular);

    // Point light properties.
    for (std::size_t i = 0; i < sl->points.size(); i++)
    {
        std::string attr_prefix{"point_lights[" + std::to_string(i) + "]."};

        if (!sl->points[i])
        {
            std::cerr << "Room::draw: SceneLighting->PointLight pointer is null.\n";
            return;
        }

        shader->set_vec3(attr_prefix + "position", sl->points[i]->position);
        shader->set_vec3(attr_prefix + "ambient", sl->points[i]->ambient);
        shader->set_vec3(attr_prefix + "diffuse", sl->points[i]->color * sl->points[i]->diffuse);
        shader->set_vec3(attr_prefix + "specular", sl->points[i]->color * sl->points[i]->specular);
        shader->set_float(attr_prefix + "constant", sl->points[i]->constant);
        shader->set_float(attr_prefix + "linear", sl->points[i]->linear);
        shader->set_float(attr_prefix + "quadratic", sl->points[i]->quadratic);
    }

    // Spotlight properties.
    if (!sl->spot)
    {
        std::cerr << "Room::draw: SceneLighting pointer is null.\n";
        return;
    }

    shader->set_vec3("spotlight.position", sl->spot->position);
    shader->set_vec3("spotlight.direction", sl->spot->direction);

    shader->set_float("spotlight.inner_cutoff", glm::cos(glm::radians(sl->spot->inner_cutoff)));
    shader->set_float("spotlight.outer_cutoff", glm::cos(glm::radians(sl->spot->outer_cutoff)));

    shader->set_vec3("spotlight.ambient", sl->spot->ambient);
    shader->set_vec3("spotlight.diffuse", sl->spot->diffuse);
    shader->set_vec3("spotlight.specular", sl->spot->specular);

    shader->set_float("spotlight.constant", sl->spot->constant);
    shader->set_float("spotlight.linear", sl->spot->linear);
    shader->set_float("spotlight.quadratic", sl->spot->quadratic);

    // // Material properties.
    // shader->set_float("material.shininess", 32.0f);

    // Set textures.
    unsigned int diffuse_num = 1;
    unsigned int specular_num = 1;

    std::size_t i = 0;
    for (i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);

        std::string num;
        std::string name = textures[i].type;
        if (name == "texture_diffuse")
            num = std::to_string(diffuse_num++);
        else if (name == "texture_specular")
            num = std::to_string(specular_num++);

        shader->set_int("material." + name + num, i);
    }

    if (depth_map_set)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, depth_map);
        shader->set_int("shadow_map", i);
    }

    // Draw mesh.
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::set_depth_map(unsigned int texture_id)
{
    depth_map = texture_id;
    depth_map_set = true;
}

#endif /* MESH_HPP */
