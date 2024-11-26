#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include <filesystem>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;
    out vec2 TexCoord;
    void main() {
        gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
        TexCoord = aTexCoord;
    }
)";

const char* fragmentShaderSource = R"(
     #version 330 core
    in vec2 TexCoord;
    out vec4 FragColor;
    uniform sampler2D ourTexture;
    uniform vec3 tintColor;
    void main() {
        vec4 texColor = texture(ourTexture, TexCoord);
        FragColor = vec4(texColor.rgb * tintColor, texColor.a);
    }
)";

unsigned int loadTexture(const char* filename) {
    std::string path = "Assets/" + std::string(filename);

    // Tarkista tiedoston olemassaolo
    std::ifstream file(path.c_str());
    if (!file.good()) {
        std::cerr << "File does not exist: " << path << std::endl;
        return 0;
    }

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        std::cout << "Texture loaded successfully: " << path << " (" << width << "x" << height << ", " << nrComponents << " channels)" << std::endl;
    }
    else {
        std::cerr << "Texture failed to load at path: " << path << std::endl;
        std::cerr << "stb_image error: " << stbi_failure_reason() << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

int main() {
    std::cout << "Current path: " << std::filesystem::current_path() << std::endl;

    // GLFW initialization
    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "Pupu Kuva", NULL, NULL);
    if (!window) {
        std::cerr << "Window creation failed" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // GLEW initialization
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW initialization failed" << std::endl;
        return -1;
    }

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    // Shader compilation and program creation
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Vertex shader compilation failed:\n" << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Fragment shader compilation failed:\n" << infoLog << std::endl;
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Set up vertex data for a quad to display the texture
    float vertices[] = {
        // positions        // texture coords
        -1.0f,  1.0f, 0.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 0.0f
    };
    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Load bunny texture
    unsigned int bunnyTexture = loadTexture("pupu.png");
    if (bunnyTexture == 0) {
        std::cerr << "Tekstuurin lataus epäonnistui, käytetään testikuvaa." << std::endl;

        unsigned char testImage[] = {
            255, 0, 0, 255,   0, 255, 0, 255,
            0, 0, 255, 255,   255, 255, 0, 255
        };

        glGenTextures(1, &bunnyTexture);
        glBindTexture(GL_TEXTURE_2D, bunnyTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, testImage);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    std::cout << "Texture ID: " << bunnyTexture << std::endl;
    std::cout << "Shader Program ID: " << shaderProgram << std::endl;

    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "ourTexture"), 0);

    // Color variables
    float yellow[] = { 1.0f, 1.0f, 0.0f };
    float red[] = { 1.0f, 0.0f, 0.0f };  // Changed from black to red
    float* currentColor = yellow;
    double lastChangeTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        // Set the background color to green
        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);  // Bright green
        glClear(GL_COLOR_BUFFER_BIT);

        // Check if 3 seconds have passed
        double currentTime = glfwGetTime();
        if (currentTime - lastChangeTime >= 3.0) {
            // Switch color between yellow and red
            currentColor = (currentColor == yellow) ? red : yellow;
            lastChangeTime = currentTime;
        }

        // Set the tint color
        glUniform3fv(glGetUniformLocation(shaderProgram, "tintColor"), 1, currentColor);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bunnyTexture);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // Check for OpenGL errors
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << err << std::endl;
        }
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}