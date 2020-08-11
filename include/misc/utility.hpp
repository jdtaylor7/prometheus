#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <filesystem>

unsigned int load_texture_from_file(const std::filesystem::path texture_path)
{
    // Create texture ID.
    unsigned int texture;
    glGenTextures(1, &texture);

    // Load texture.
    int width;
    int height;
    int num_channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(texture_path.c_str(), &width, &height, &num_channels, 0);
    if (data)
    {
        GLenum format;
        if (num_channels == 1)
            format = GL_RED;
        else if (num_channels == 3)
            format = GL_RGB;
        else if (num_channels == 4)
            format = GL_RGBA;

        // Generate texture.
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Set texture parameters.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cerr << "Failed to load texture at " << texture_path << '\n';
        stbi_image_free(data);
    }

    return texture;
}

#endif /* UTILITY_HPP */
