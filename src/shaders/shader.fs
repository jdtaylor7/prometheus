#version 330 core

in vec2 vert_tex_coords;

out vec4 frag_color;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform vec3 in_color;

void main()
{
    // frag_color = mix(texture(texture1, vert_tex_coords),
    //                  texture(texture2, vert_tex_coords),
    //                  0.4);
    frag_color = vec4(in_color.x, in_color.y, in_color.z, 1.0);
}
