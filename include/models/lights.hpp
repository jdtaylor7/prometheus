#ifndef LIGHTS_HPP
#define LIGHTS_HPP

#include <memory>
#include <vector>

// TODO remove
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "shapes.hpp"

struct DirectionalLight;
class PointLight;
struct Spotlight;

struct SceneLighting
{
    SceneLighting(DirectionalLight* dir_,
        std::vector<std::shared_ptr<PointLight>> points_,
        Spotlight* spot_) :
            dir(dir_), points(points_), spot(spot_)
    {
    }

    DirectionalLight* dir = nullptr;
    std::vector<std::shared_ptr<PointLight>> points;
    Spotlight* spot = nullptr;
};

struct DirectionalLight
{
    DirectionalLight(glm::vec3 direction_,
        glm::vec3 ambient_,
        glm::vec3 diffuse_,
        glm::vec3 specular_) :
            direction(direction_),
            ambient(ambient_),
            diffuse(diffuse_),
            specular(specular_)
    {
    }

    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

class PointLight
{
public:
    PointLight(
        glm::vec3 position_,
        glm::vec3 color_,
        float scale_factor_,
        glm::vec3 ambient_,
        glm::vec3 diffuse_,
        glm::vec3 specular_,
        float constant_,
        float linear_,
        float quadratic_) :
            position(position_),
            color(color_),
            scale_factor(scale_factor_),
            ambient(ambient_),
            diffuse(diffuse_),
            specular(specular_),
            constant(constant_),
            linear(linear_),
            quadratic(quadratic_)
    {
    }

    void init();
    void deinit();
    void draw();

    glm::vec3 position;
    glm::vec3 color;
    float scale_factor;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
private:
    unsigned int vao;
    unsigned int vbo;
};

void PointLight::init()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * cube_vertices.size(), cube_vertices.data(), GL_STATIC_DRAW);

    // Specify vertex data format.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void PointLight::deinit()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

void PointLight::draw()
{
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

struct Spotlight
{
    Spotlight(
        glm::vec3 position_,
        glm::vec3 direction_,
        float inner_cutoff_,
        float outer_cutoff_,
        glm::vec3 ambient_,
        glm::vec3 diffuse_,
        glm::vec3 specular_,
        float constant_,
        float linear_,
        float quadratic_) :
            position(position_),
            direction(direction_),
            inner_cutoff(inner_cutoff_),
            outer_cutoff(outer_cutoff_),
            ambient(ambient_),
            diffuse(diffuse_),
            specular(specular_),
            constant(constant_),
            linear(linear_),
            quadratic(quadratic_)
    {
    }

    void update(glm::vec3 pos, glm::vec3 dir)
    {
        position = pos;
        direction = dir;
    }

    glm::vec3 position;
    glm::vec3 direction;

    float inner_cutoff;
    float outer_cutoff;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

#endif /* LIGHTS_HPP */
