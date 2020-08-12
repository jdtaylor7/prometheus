#ifndef SHADER_HPP
#define SHADER_HPP

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "logger.hpp"

namespace fs = std::filesystem;

class Shader
{
public:
    Shader() : vertex_path{}, fragment_path{} {}
    Shader(const fs::path& vpath, const fs::path& fpath) :
        vertex_path(vpath), fragment_path(fpath) {}

    void init();
    void use();

    void set_bool(const std::string& name, bool value) const;
    void set_int(const std::string& name, int value) const;
    void set_float(const std::string& name, float value) const;
    void set_vec3(const std::string& name, const glm::vec3& v);
    void set_mat4fv(const std::string& name, const glm::mat4& m);
private:
    fs::path vertex_path;
    fs::path fragment_path;
    unsigned int id;
};

void Shader::init()
{
    std::string vertex_code;
    std::string fragment_code;
    std::ifstream vertex_shader_file;
    std::ifstream fragment_shader_file;

    vertex_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fragment_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        vertex_shader_file.open(vertex_path);
        fragment_shader_file.open(fragment_path);

        std::stringstream vertex_shader_stream;
        std::stringstream fragment_shader_stream;

        vertex_shader_stream << vertex_shader_file.rdbuf();
        fragment_shader_stream << fragment_shader_file.rdbuf();

        vertex_shader_file.close();
        fragment_shader_file.close();

        vertex_code = vertex_shader_stream.str();
        fragment_code = fragment_shader_stream.str();
    }
    catch (std::ifstream::failure e)
    {
        logger.log(LogLevel::error, "Shader::init: Shader file could not be read: ", e.what(), '\n');
    }

    const char* vertex_shader_source = vertex_code.c_str();
    const char* fragment_shader_source = fragment_code.c_str();

    int success;
    char info_log[512];

    // Create vertex shader object.
    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);

    // Check for vertex shader compilation errors.
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
        logger.log(LogLevel::error, "Shader::init: Vertex shader could not be compiled: ", info_log, '\n');
    }

    // Create fragment shader object.
    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    // Check for fragment shader compilation errors.
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
        logger.log(LogLevel::error, "Shader::init: Fragment shader could not be compiled: ", info_log, '\n');
    }

    // Build shader program.
    id = glCreateProgram();
    glAttachShader(id, vertex_shader);
    glAttachShader(id, fragment_shader);
    glLinkProgram(id);

    // Check for shader program link errors.
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(id, 512, NULL, info_log);
        logger.log(LogLevel::error, "Shader::init: Shader file could not be linked: ", info_log, '\n');
    }

    // Delete the shader objects since they've already been linked into the
    // shader program.
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

void Shader::use()
{
    glUseProgram(id);
}

void Shader::set_bool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
}

void Shader::set_int(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::set_float(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::set_vec3(const std::string& name, const glm::vec3& v)
{
    glUniform3f(glGetUniformLocation(id, name.c_str()), v.x, v.y, v.z);
}

void Shader::set_mat4fv(const std::string& name, const glm::mat4& m)
{
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(m));
}

#endif /* SHADER_HPP */
