#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <random>
#include "Mesh.h"
#include "Shader.h"
#include "Light.h"

// REMEMBER ITS TO THE POWER OF 2, NOT DIVISIBLE BY 2 (2^n+1)
#define MAP_SIZE 5

const char *vertShaderSource = R"(
#version 330 core

layout(location = 0) in vec3 aPos;
// layout(location = 1) in vec3 aNormal;
// layout(location = 2) in vec2 aUv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 normal;
out vec2 uv;

void main() {
    // normal = aNormal;
    // uv = aUv;
    gl_Position = projection * view * model * vec4(aPos, 1.f);
}
)";

const char *fragShaderSource = R"(
#version 330 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 emissive;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
}

// in vec3 normal;
// in vec2 uv;

uniform vec3 globalAmbient;
uniform Material material;
uniform Light light;

out vec4 colour;

void main() {
    vec3 ambient = globalAmbient * material.ambient * light.ambient;

    colour = vec4(ambient, 1);
}
)";

std::default_random_engine generator;
glm::vec3 globalAmbient(.2f, .2f, .2f);
Shader *shader;

struct Camera {
    glm::vec3 position = glm::vec3(0.f, 0.f, 0.f);
    glm::vec3 direction = glm::vec3(0.f, 0.f, -1.f);
    glm::mat4 viewMatrix = glm::mat4(1.f);
    glm::mat4 projMatrix = glm::mat4(1.f);
    // Used for rotation delta
    double cursorPosLastX = 0;
    double cursorPosLastY = 0;
    float yaw = 0.f;
    float pitch = 0.f;
    // Camera settings
    float fov = 90.f;
    float near = .1f;
    float far = 1000.f;
    float translateSpeed = .1f;
    float rotationSpeed = .25f;

    void updateProjectionMatrix(const int width, const int height) {
        projMatrix = glm::perspective(glm::radians(fov), (float) width / (float) height, near, far);
        shader->setUniform("projection", projMatrix);
    }

    void updateViewMatrix() {
        // Prevent "flipping" the camera when looking up and down
        pitch = glm::clamp(pitch, -89.9f, 89.9f);

        // Calculate looking direction
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(-pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction = glm::normalize(direction);

        viewMatrix = glm::lookAt(position, position + direction, glm::vec3(0.f, 1.f, 0.f));
        shader->setUniform("view", viewMatrix);
    }

} camera;

const Light light {
    glm::vec3(1.f, 1.f, 0.f),
    glm::vec3(0.f),
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

void glfwCursorPosCallback(GLFWwindow *window, double xPos, double yPos) {
    auto cursorDelta = glm::dvec2(xPos - camera.cursorPosLastX, yPos - camera.cursorPosLastY);

    camera.yaw += cursorDelta.x * camera.rotationSpeed;
    camera.pitch += cursorDelta.y * camera.rotationSpeed;
    camera.updateViewMatrix();

    camera.cursorPosLastX = xPos;
    camera.cursorPosLastY = yPos;
}

void glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // Repeating key used for camera controls
    // todo translate based on facing position
    if (action == GLFW_REPEAT) {
        switch (key) {
            case GLFW_KEY_W:
                camera.position.z -= camera.translateSpeed;
                break;
            case GLFW_KEY_S:
                camera.position.z += camera.translateSpeed;
                break;
            case GLFW_KEY_A:
                camera.position.x -= camera.translateSpeed;
                break;
            case GLFW_KEY_D:
                camera.position.x += camera.translateSpeed;
                break;
            case GLFW_KEY_R:
                camera.position.y += camera.translateSpeed;
                break;
            case GLFW_KEY_F:
                camera.position.t -= camera.translateSpeed;
                break;
            case GLFW_KEY_Q:
                camera.direction.z -= camera.rotationSpeed;
                break;
            case GLFW_KEY_E:
                camera.direction.z += camera.rotationSpeed;
                break;
        }
        camera.updateViewMatrix();
    }
}

void glfwFramebufferSizeCallback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    camera.updateProjectionMatrix(width, height);
}

float randomInRange(float min, float max) {
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(generator);
}

float diamondStep(Mesh *mesh, int x, int y, int stepSize) {
    float averageHeight = 0.f;
    int xMin = x - stepSize;
    int xMax = x + stepSize;
    int yMin = y - stepSize;
    int yMax = y + stepSize;
    averageHeight += mesh->getValue(xMin, yMin).y; // Top left
    averageHeight += mesh->getValue(xMin, yMax).y; // Bottom left
    averageHeight += mesh->getValue(xMax, yMin).y; // Top right
    averageHeight += mesh->getValue(xMax, yMax).y; // Bottom right
    return averageHeight / 4.f;
}

// todo wrap around
/**
 * Calculates the average height for the provided vertex based on a diamond pattern around it
 * @param vertices
 * @param x
 * @param y
 * @param stepSize
 */
float squareStep(Mesh *vertices, int x, int y, int stepSize) {
    float averageHeight = 0.f;
    int count = 0;
    int xMin = x - stepSize;
    int xMax = x + stepSize;
    int yMin = y - stepSize;
    int yMax = y + stepSize;
    if (xMin >= 0) {
        averageHeight += vertices->getValue(xMin, y).y; // Left
        count++;
    }
    if (xMax < MAP_SIZE) {
        averageHeight += vertices->getValue(xMax, y).y; // Right
        count++;
    }
    if (yMin >= 0) {
        averageHeight += vertices->getValue(x, yMin).y; // Top
        count++;
    }
    if (yMax < MAP_SIZE) {
        averageHeight += vertices->getValue(x, yMax).y; // Bottom
        count++;
    }
    return averageHeight / count;
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

    for (int x = halfStepSize; x < MAP_SIZE - 1; x += stepSize) {
        for (int y = halfStepSize; y < MAP_SIZE - 1; y += stepSize) {
            mesh->getValue(x, y).y = diamondStep(mesh, x, y, halfStepSize) + randomInRange(-randMax, randMax);
        }
    }

    bool offset = false;
    for (int x = 0; x <= MAP_SIZE - 1; x += halfStepSize) {
        offset = !offset;
        for (int y = offset ? halfStepSize : 0; y <= MAP_SIZE - 1; y += stepSize) {
            mesh->getValue(x, y).y = squareStep(mesh, x, y, halfStepSize) + randomInRange(-randMax, randMax);
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
    float maxRand = 1.f;
    float h = 1.f;

    mesh->getValue(0, 0).y = randomInRange(-maxRand, maxRand);
    mesh->getValue(0, MAP_SIZE - 1).y = randomInRange(-maxRand, maxRand);
    mesh->getValue(MAP_SIZE - 1, MAP_SIZE - 1).y = randomInRange(-maxRand, maxRand);
    mesh->getValue(MAP_SIZE - 1, 0).y = randomInRange(-maxRand, maxRand);
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
    shader = new Shader(vertShaderSource, fragShaderSource);
    shader->use();
    shader->setGlobalAmbient(globalAmbient);
    shader->setLight(light);

    glfwSetFramebufferSizeCallback(window, glfwFramebufferSizeCallback);
    glfwSetKeyCallback(window, glfwKeyCallback);
    glfwSetCursorPosCallback(window, glfwCursorPosCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Lock mouse to window and hide cursor

    // Set some default parameters
    glClearColor(.7f, .7f, .7f, 1.f);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    // Generate terrain
    std::vector<Mesh *> terrain;
    generateTerrain(terrain);

    glViewport(0, 0, 1080, 720);
    camera.updateProjectionMatrix(1080, 720);
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