#include <iostream>
#include <string>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.hpp"
#include "shader.hpp"

constexpr std::size_t SCREEN_WIDTH = 1200;
constexpr std::size_t SCREEN_HEIGHT = 900;

const std::string vertex_shader_path = "src/shaders/shader.vs";
const std::string fragment_shader_path = "src/shaders/shader.fs";

const std::string container_texture_path = "include/textures/container.jpg";
const std::string face_texture_path = "include/textures/awesomeface.png";
const std::string wall_texture_path = "include/textures/wall.png";

glm::vec3 room_color = glm::vec3(1.0, 1.0, 1.0);
glm::vec3 drone_color = glm::vec3(1.0, 0.5, 0.2);

constexpr float room_size = 10.0f;
glm::vec3 room_pos(0.0, 4.5, 0.0);

Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT, room_size / 2);

const std::vector<float> vertices = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

const std::vector<unsigned int> indices = {
    0, 1, 3,  // right triangle
    1, 2, 3,  // left triangle
};

glm::vec3 drone_pos(0.0, 0.0, 0.0);

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void process_input(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    camera.update_pos(window);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    camera.update_angle(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.update_pov(yoffset);
}

int main()
{
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;

    /*
     * GLFW initialization and configuration.
     */
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /*
     * GLFW window creation.
     */
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello Triangle", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    /*
     * Load OpenGL function pointers.
     */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    /*
     * Build buffers and vertex array object.
     */

    // Generate and bind a "vertex array object" (VAO) to store the VBO and
    // corresponding vertex attribute configurations.
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Send vertex data to the vertex shader. Do so by allocating GPU
    // memory, which is managed by "vertex buffer objects" (VBOs).
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind VBO to the vertex buffer object, GL_ARRAY_BUFFER. Buffer
    // operations on GL_ARRAY_BUFFER then apply to VBO.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    // Bind EBO to the element buffer object, GL_ELEMENT_ARRAY_BUFFER. Buffer
    // operations on GL_ELEMENT_ARRAY_BUFFER then apply to EBO.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

    // Specify vertex data format.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Can now unbind VAO and VBO, will rebind VAO as necessary in render loop.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    /*
     * Create shader program.
     */
    Shader shader(vertex_shader_path, fragment_shader_path);

    /*
     * Create textures.
     */

    // Create texture ID.
    std::vector<unsigned int> textures(2);
    glGenTextures(2, textures.data());

    // Activate a texture unit and bind the texture as the current GL_TEXTURE_2D.
    glBindTexture(GL_TEXTURE_2D, textures[0]);

    // Set texture wrapping and filtering options.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load container texture.
    int width;
    int height;
    int num_channels;
    unsigned char *data = stbi_load(container_texture_path.c_str(), &width, &height, &num_channels, 0);

    // Generate texture.
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load texture\n";
    }

    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load and generate face texture.
    stbi_set_flip_vertically_on_load(true);
    data = stbi_load(face_texture_path.c_str(), &width, &height, &num_channels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load texture\n";
    }

    // Set uniforms.
    shader.use();
    // std::cout << "texture1 = " << textures[0] << '\n';
    // std::cout << "texture2 = " << textures[1] << '\n';
    // shader.set_int("texture1", textures[0]);
    // shader.set_int("texture2", textures[1]);
    // shader.set_int("texture1", 0);
    // shader.set_int("texture2", 1);

    glEnable(GL_DEPTH_TEST);

    glm::mat4 room_model = glm::mat4(1.0f);
    room_model = glm::translate(room_model, room_pos);
    room_model = glm::scale(room_model, glm::vec3(room_size, room_size, room_size));

    /*
     * Render loop.
     */
    while (!glfwWindowShouldClose(window))
    {
        camera.update_frames();

        /*
         * Input.
         */
        process_input(window);

        /*
         * Render.
         */

        // Color buffer.
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render square.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textures[1]);

        // Create transformation matrices.
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
        view = glm::lookAt(camera.get_pos(), camera.get_pos() + camera.get_front(), camera.get_up());
        projection = glm::perspective(glm::radians(camera.get_fov()), 800.0f / 600.0f, 0.1f, 100.0f);

        shader.use();
        // shader.set_int("texture1", 0);
        // shader.set_int("texture2", 0);
        shader.set_vec3("in_color", room_color);
        shader.set_mat4fv("model", model);
        shader.set_mat4fv("view", view);
        shader.set_mat4fv("projection", projection);
        glBindVertexArray(VAO);

        // Draw room.
        shader.set_mat4fv("model", room_model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Draw drone.
        // shader.set_int("texture1", 1);
        // shader.set_int("texture2", 1);
        shader.set_vec3("in_color", drone_color);
        model = glm::mat4(1.0f);
        model = glm::translate(model, drone_pos);
        model = glm::scale(model, glm::vec3(0.2, 0.2, 0.2));
        shader.set_mat4fv("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);

        /*
         * Swap buffers and poll I/O events.
         */
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    /*
     * Clean up.
     */
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}
