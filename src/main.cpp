#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <random>
#include <stb_image.h>
#include "Mesh.h"
#include "Shader.h"
#include "Light.h"
#include "Camera.h"

// REMEMBER ITS TO THE POWER OF 2, NOT DIVISIBLE BY 2 (2^n+1)
#define MAP_SIZE 33

std::default_random_engine generator;
glm::vec3 globalAmbient(.2f, .2f, .2f);
Shader *shader;
Camera camera;

const Light light {
    glm::vec3(2.5f, 10.f, 2.5f),
    glm::vec3(1.f),
    glm::vec3(1.f),
    glm::vec3(1.f),
};

void glErrorCheck() {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "(" << __FILE__ << ":" << __LINE__ << ") OpenGL error 0x" << std::hex << err << std::endl;
    }
}

void glfwErrorCallback(int errCode, const char *description) {
    std::cerr << "GLFW Error " << errCode << ": " << description << std::endl;
}

void glfwFramebufferSizeCallback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    camera.updateProjectionMatrix(width, height);
    shader->setUniform("projection", camera.getProjMatrix());
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(imageData);
    return textureId;
}

float diamondStep(Mesh *mesh, int x, int y, int stepSize) {
    float averageHeight = 0.f;
    int xMin = x - stepSize;
    int xMax = x + stepSize;
    int yMin = y - stepSize;
    int yMax = y + stepSize;
    averageHeight += mesh->getValue(xMin, yMin).position.y; // Top left
    averageHeight += mesh->getValue(xMin, yMax).position.y; // Bottom left
    averageHeight += mesh->getValue(xMax, yMin).position.y; // Top right
    averageHeight += mesh->getValue(xMax, yMax).position.y; // Bottom right
    return averageHeight / 4.f;
}

/**
 * Calculates the average height for the provided vertex based on a diamond pattern around it
 * @param vertices
 * @param x
 * @param y
 * @param stepSize
 */
float squareStep(Mesh *vertices, int x, int y, int stepSize) {
    float averageHeight = 0.f;
    int xMin = x - stepSize;
    int xMax = x + stepSize;
    int yMin = y - stepSize;
    int yMax = y + stepSize;
    if (xMin < 0) {
        xMin = MAP_SIZE - abs(xMin);
    }
    averageHeight += vertices->getValue(xMin, y).position.y; // Left
    if (xMax >= MAP_SIZE) {
        xMax = xMax - MAP_SIZE;
    }
    averageHeight += vertices->getValue(xMax, y).position.y; // Right
    if (yMin < 0) {
        yMin = MAP_SIZE - abs(yMin);
    }
    averageHeight += vertices->getValue(x, yMin).position.y; // Top
    if (yMax >= MAP_SIZE) {
        yMax = yMax - MAP_SIZE;
    }
    averageHeight += vertices->getValue(x, yMax).position.y; // Bottom
    return averageHeight / 4.f;
}

/**
 * Applies the Diamond-Square algorithm to the provided set of vertices in a recursive way
 * @param mesh The mesh data
 * @param h The smoothness
 * @param stepSize The step size
 * @param randMax Maximum random offset
 */
void diamondSquare(Mesh *mesh, float h, int stepSize, float randMax) {
    if (stepSize <= 1) return;
    int halfStepSize = stepSize / 2;
    std::uniform_real_distribution<float> distribution(-randMax, randMax);

    for (int x = halfStepSize; x < MAP_SIZE - 1; x += stepSize) {
        for (int y = halfStepSize; y < MAP_SIZE - 1; y += stepSize) {
            mesh->getValue(x, y).position.y = diamondStep(mesh, x, y, halfStepSize) + distribution(generator);
        }
    }

    bool offset = false;
    for (int x = 0; x <= MAP_SIZE - 1; x += halfStepSize) {
        offset = !offset;
        for (int y = offset ? halfStepSize : 0; y <= MAP_SIZE - 1; y += stepSize) {
            mesh->getValue(x, y).position.y = squareStep(mesh, x, y, halfStepSize) + distribution(generator);
        }
    }

    randMax = randMax * powf(2, -h);
    stepSize = halfStepSize;
    diamondSquare(mesh, h, stepSize, randMax);
}

void generateTerrain(std::vector<Mesh *> &terrain) {
    Material material = {
            glm::vec3(1.f, 1.f, 1.f),
            glm::vec3(1.f, 1.f, 1.f),
            glm::vec3(1.f, 1.f, 1.f),
            glm::vec3(1.f, 1.f, 1.f),
            0.f
    };
    auto mesh = new Mesh(MAP_SIZE, MAP_SIZE, material);
    float maxRand = 7.f;
    float h = 1.f;
    std::uniform_real_distribution<float> distribution(-maxRand, maxRand);
    float cornerStart = distribution(generator);

    mesh->getValue(0, 0).position.y = cornerStart;
    mesh->getValue(0, MAP_SIZE - 1).position.y = cornerStart;
    mesh->getValue(MAP_SIZE - 1, MAP_SIZE - 1).position.y = cornerStart;
    mesh->getValue(MAP_SIZE - 1, 0).position.y = cornerStart;
    diamondSquare(mesh, h, MAP_SIZE - 1, maxRand);

    mesh->buildBuffers();
    terrain.push_back(mesh);
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

    // Generate shaders
    shader = new Shader("assets/shaders/vert.glsl", "assets/shaders/terrain_frag.glsl");
    shader->use();
    shader->setGlobalAmbient(globalAmbient);
    shader->setLight(light);

    // Setup callbacks
    glfwSetFramebufferSizeCallback(window, glfwFramebufferSizeCallback);
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        camera.handleKey(key, scancode, action, mods);
        shader->setUniform("view", camera.getViewMatrix());
    });
    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xPos, double yPos) {
        camera.handleCursorMove(xPos, yPos);
        shader->setUniform("view", camera.getViewMatrix());
    });
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Lock mouse to window and hide cursor

    // Set some default parameters
    glClearColor(.7f, .7f, .7f, 1.f);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    // Generate terrain
    std::vector<Mesh *> terrain;
    generateTerrain(terrain);

    // Load and bind terrain texture TODO temp
    auto grassyRockTexture = loadTexture("assets/textures/rock-grassy.jpg");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, grassyRockTexture);

    // Initialise camera
    glViewport(0, 0, 1080, 720);
    camera.updateProjectionMatrix(1080, 720);
    shader->setUniform("projection", camera.getProjMatrix());
    camera.updateViewMatrix();

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto mesh : terrain) {
            mesh->render(shader);
        }

        glErrorCheck();

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    return 0;
}