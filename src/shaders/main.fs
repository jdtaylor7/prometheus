#version 330 core

struct PointLight
{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    // Attenuation parameters.
    float constant;
    float linear;
    float quadratic;
};

struct Material
{
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float shininess;
};

#define NUM_POINT_LIGHTS 1

in vec3 frag_pos;
in vec3 normal_vec;
in vec2 tex_coords;
in vec4 frag_pos_light_space;

uniform vec3 view_pos;
uniform PointLight point_lights[NUM_POINT_LIGHTS];
uniform Material material;
uniform sampler2D shadow_map;
uniform bool smooth_shadows;

out vec4 frag_color;

float calc_shadow(vec3 normal, vec3 light_dir)
{
    // Normalize perspective.
    vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;

    // Transform from clip space ([-1, 1]) to screen space ([0, 1]).
    proj_coords = (proj_coords * 0.5f) + 0.5f;

    // Compute closest and current depths.
    float closest_depth = texture(shadow_map, proj_coords.xy).r;
    float current_depth = proj_coords.z;

    // Provide bias to shadow calculations to remove shadow acne.
    float shadow_bias = 0.0005f;

    // Calculate whether fragment is in shadow. Implement PCF by averaging
    // surrounding texels.
    float shadow = 0.0f;
    if (smooth_shadows)
    {
        vec2 texel_size = 1.0f / textureSize(shadow_map, 0);
        for (int x = -2; x <= 2; ++x)
        {
            for (int y = -2; y <= 2; ++y)
            {
                float pcf_depth = texture(shadow_map, proj_coords.xy + vec2(x, y) * texel_size).r;
                shadow += current_depth - shadow_bias > pcf_depth ? 0.075f : 0.0f;
            }
        }
    }
    else
    {
        shadow = current_depth - shadow_bias > closest_depth ? 1.0f : 0.0f;
    }

    // Remove shadows outside of light frustum.
    if (proj_coords.z > 1.0f)
        shadow = 0.0f;

    return shadow;
    // return closest_depth;
    // return current_depth;
}

vec3 calc_point_light(PointLight light, vec3 normal, vec3 frag_pos, vec3 view_dir)
{
    // Ambient.
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, tex_coords));

    // Diffuse.
    vec3 light_dir = normalize(light.position - frag_pos);
    float diff = max(dot(normal, light_dir), 0.0f);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, tex_coords));

    // Specular.
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0f), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, tex_coords));

    // Attenuation.
    float distance = length(light.position - frag_pos);
    float attenuation = 1.0f / (light.constant + \
                               (light.linear * distance) + \
                               (light.quadratic * distance * distance));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    // Shadow.
    float shadow = calc_shadow(normal, light_dir);

    return (ambient + (1.0f - shadow) * (diffuse + specular));
    // return vec3(1.0f - shadow);
    // return vec3(shadow);
}

void main()
{
    // Precomputed values for light calcs.
    vec3 normal = normalize(normal_vec);
    vec3 view_dir = normalize(view_pos - frag_pos);

    vec3 result = vec3(0.0f);

    // Point lights.
    for (int i = 0; i < NUM_POINT_LIGHTS; i++)
        result += calc_point_light(point_lights[i], normal, frag_pos, view_dir);

    frag_color = vec4(result, 1.0f);
}
