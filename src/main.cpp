#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <random>
#include <stb_image.h>
#include "Terrain.h"
#include "Shader.h"
#include "Light.h"
#include "Camera.h"
#include "Skybox.h"
#include "glHelper.h"

// REMEMBER ITS TO THE POWER OF 2, NOT DIVISIBLE BY 2 (2^n+1)
#define MAP_SIZE 33

glm::vec3 globalAmbient(.2f, .2f, .2f);
Camera camera;
std::vector<Shader *> shaders;

const Light light {
    glm::vec3(2.5f, 10.f, 2.5f),
    glm::vec3(1.f),
    glm::vec3(1.f),
    glm::vec3(1.f),
};

void glfwErrorCallback(int errCode, const char *description) {
    std::cerr << "GLFW Error " << errCode << ": " << description << std::endl;
}

void glfwFramebufferSizeCallback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    camera.updateProjectionMatrix(width, height);
    for (auto shader : shaders) {
        shader->use();
        shader->setUniform("projection", camera.getProjMatrix());
    }
}

/**
 * Loads a texture from file into OpenGL
 * @param filePath Path to the texutre
 * @return OpenGL id for the texture. 0 if failed to load
 */
GLuint loadTexture(const char *filePath) {
    GLuint textureId;
    int width, height, channels;
    auto imageData = stbi_load(filePath, &width, &height, &channels, 4);

    if (imageData == nullptr) {
        std::cerr << "Failed to load texture file: " << filePath << std::endl;
        return 0;
    }

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(imageData);
    return textureId;
}

void generateTerrain(std::vector<Terrain *> &terrains) {
    // Main terrain
    auto shader = new Shader("assets/shaders/vert.glsl", "assets/shaders/terrain_frag.glsl");
    shader->setGlobalAmbient(globalAmbient);
    shader->setLight(light);
    shaders.push_back(shader);
    GLERRCHECK();

    Material material = {
            glm::vec3(1.f, 1.f, 1.f),
            glm::vec3(1.f, 1.f, 1.f),
            glm::vec3(1.f, 1.f, 1.f),
            glm::vec3(1.f, 1.f, 1.f),
            0.f,
            {
                loadTexture("assets/textures/sand.jpg"),
                loadTexture("assets/textures/grass.jpg")
            }
    };
    terrains.push_back(new Terrain(MAP_SIZE, 7.f, 1.f, shader, material));
    GLERRCHECK();

    // Water
    // Main terrain
    auto waterShader = new Shader("assets/shaders/water_vert.glsl", "assets/shaders/water_frag.glsl");
    waterShader->setGlobalAmbient(globalAmbient);
    waterShader->setLight(light);
    shaders.push_back(waterShader);
    GLERRCHECK();

    Material waterMaterial = {
            glm::vec3(1.f, 1.f, 1.f),
            glm::vec3(1.f, 1.f, 1.f),
            glm::vec3(1.f, 1.f, 1.f),
            glm::vec3(1.f, 1.f, 1.f),
            0.f,
            {
                    loadTexture("assets/textures/water.jpg")
            }
    };
    auto water = new Terrain(MAP_SIZE, 1.f, .8f, waterShader, waterMaterial);
    water->setPosition(glm::vec3(0.f, -2.f, 0.f));
    terrains.push_back(water);
    GLERRCHECK();
}

int main() {
    glfwSetErrorCallback(glfwErrorCallback);

    GLFWwindow *window;

    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
#endif

    window = glfwCreateWindow(1080, 720, "322COM ProcGen", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW Window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to load GLAD!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwSwapInterval(1);

    // Setup callbacks
    glfwSetFramebufferSizeCallback(window, glfwFramebufferSizeCallback);
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        camera.handleKey(key, scancode, action, mods);
        for (auto shader : shaders) {
            shader->use();
            shader->setUniform("view", camera.getViewMatrix());
        }
    });
    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xPos, double yPos) {
        camera.handleCursorMove(xPos, yPos);
        for (auto shader : shaders) {
            shader->use();
            shader->setUniform("view", camera.getViewMatrix());
        }
    });
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Lock mouse to window and hide cursor

    // Set some default parameters
    glClearColor(.7f, .7f, .7f, 1.f);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    GLERRCHECK();

    // Load skybox
    auto skybox = new Skybox(new Shader("assets/shaders/skybox_vert.glsl", "assets/shaders/skybox_frag.glsl"), std::string("assets/textures/skybox_"));
    GLERRCHECK();

    // Generate terrain
    std::vector<Terrain *> terrain;
    generateTerrain(terrain);

    // Initialise camera
    glViewport(0, 0, 1080, 720);
    camera.updateProjectionMatrix(1080, 720);
    camera.updateViewMatrix();
    for (auto shader : shaders) {
        shader->use();
        shader->setUniform("projection", camera.getProjMatrix());
        GLERRCHECK();
    }

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        skybox->render(camera);
        GLERRCHECK();

        for (auto mesh : terrain) {
            mesh->render();
            GLERRCHECK();
        }

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    return 0;
}