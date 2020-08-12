#version 330 core

out vec4 frag_color;

in vec2 tex_coords;

uniform sampler2D depth_map;
uniform float near_plane;
uniform float far_plane;

float linearize_depth(float depth)
{
    float z = depth * 2.0f - 1.0f;
    return (2.0f * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{
    // Perspective.
    float depth_value = texture(depth_map, tex_coords).r;
    frag_color = vec4(vec3(linearize_depth(depth_value) / far_plane), 1.0f);

    // Orthographic.
    // frag_color = vec4(vec3(depth_value), 1.0f);
}
