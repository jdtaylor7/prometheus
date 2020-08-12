#version 330 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_tex_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 light_space_matrix;

out vec3 frag_pos;
out vec3 normal_vec;
out vec2 tex_coords;
out vec4 frag_pos_light_space;

void main()
{
    gl_Position = projection * view * model * vec4(in_pos, 1.0f);
    frag_pos = vec3(model * vec4(in_pos, 1.0f));
    normal_vec = mat3(transpose(inverse(model))) * in_normal;
    tex_coords = in_tex_coords;
    frag_pos_light_space = light_space_matrix * vec4(frag_pos, 1.0f);
}
