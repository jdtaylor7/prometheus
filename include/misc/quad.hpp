#ifndef QUAD_HPP
#define QUAD_HPP

#include <vector>

#include <GLFW/glfw3.h>

std::vector<float> quad_vertices = {
    // positions        // tex_coords
    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
     1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
     1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
};

class Quad
{
public:
    void init();
    void draw(Shader*);

    void set_depth_map(unsigned int depth_map_) { depth_map = depth_map_; }
private:
    unsigned int vao;
    unsigned int vbo;
    unsigned int depth_map;
};

void Quad::init()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * quad_vertices.size(), quad_vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}

void Quad::draw(Shader* shader)
{
    if (!shader)
    {
        std::cerr << "Quad::draw: shader is NULL\n";
        return;
    }

    shader->use();

    // Bind vertex buffers.
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * quad_vertices.size(), quad_vertices.data(), GL_STATIC_DRAW);

    // Bind textures.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depth_map);
    shader->set_int("depth_map", 0);

    // Render.
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

#endif /* QUAD_HPP */
