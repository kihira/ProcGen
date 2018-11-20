#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#define MAP_SIZE 5

struct Camera {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;
    // Used for rotation delta
    double cursorPosLastX;
    double cursorPosLastY;
    // Camera settings
    float fov = 90.f;
    float near = .1f;
    float far = 1000.f;
    float translateSpeed = .1f;
    float rotationSpeed = .1f;

    void updateProjectionMatrix(const int width, const int height) {
        projMatrix = glm::perspective(fov, (float) width / height, near, far);
        // todo update shader uniform
    }

    void updateViewMatrix() {
        viewMatrix = glm::translate(glm::mat4(1.f), position);
        viewMatrix = glm::rotate(viewMatrix, rotation.x, glm::vec3(1.f, 0.f, 0.f));
        viewMatrix = glm::rotate(viewMatrix, rotation.y, glm::vec3(0.f, 1.f, 0.f));
        viewMatrix = glm::rotate(viewMatrix, rotation.z, glm::vec3(0.f, 0.f, 1.f));
        // todo update shader uniform
    }

} camera;

void glfwCursorPosCallback(GLFWwindow *window, double xPos, double yPos) {
    auto cursorDelta = glm::dvec2(xPos - camera.cursorPosLastX, yPos - camera.cursorPosLastY);
    camera.rotation.x += cursorDelta.y * camera.rotationSpeed;
    camera.rotation.y += cursorDelta.x * camera.rotationSpeed;
    camera.updateViewMatrix();

    camera.cursorPosLastX = xPos;
    camera.cursorPosLastY = yPos;
}

void glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // Repeating key used for camera controls
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
                camera.rotation.z -= camera.rotationSpeed;
                break;
            case GLFW_KEY_E:
                camera.rotation.z += camera.rotationSpeed;
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
    return min + static_cast<float>(rand()) / static_cast<float>(RAND_MAX)/(max - min);
}

// todo wrap around
float diamondStep(glm::vec3 vertices[MAP_SIZE][MAP_SIZE], int x, int y, int stepSize) {
    float averageHeight = 0.f;
    int count = 0;
    int xMin, xMax, yMin, yMax = 0;
    xMin = x - stepSize;
    xMax = x + stepSize;
    yMin = y - stepSize;
    yMax = y + stepSize;
    if (xMin >= 0) {
        if (yMin >= 0) {
            averageHeight += vertices[xMin][yMin].y; // Top left
            count++;
        }
        if (yMax < MAP_SIZE) {
            averageHeight += vertices[xMin][yMax].y; // Bottom left
            count++;
        }
    }
    if (xMax < MAP_SIZE) {
        if (yMin >= 0) {
            averageHeight += vertices[xMax][yMin].y; // Top right
            count++;
        }
        if (yMax < MAP_SIZE) {
            averageHeight += vertices[xMax][yMax].y; // Bottom right
            count++;
        }
    }
    return averageHeight / count;
}

// todo wrap around
/**
 * Calculates the average height for the provided vertex based on a diamond pattern around it
 * @param vertices
 * @param x
 * @param y
 * @param stepSize
 */
float squareStep(glm::vec3 vertices[MAP_SIZE][MAP_SIZE], int x, int y, int stepSize) {
    float averageHeight = 0.f;
    int count = 0;
    int xMin, xMax, yMin, yMax = 0;
    xMin = x - stepSize;
    xMax = x + stepSize;
    yMin = y - stepSize;
    yMax = y + stepSize;
    if (xMin >= 0) {
        averageHeight += vertices[xMin][y].y; // Left
        count++;
    }
    if (xMax < MAP_SIZE) {
        averageHeight += vertices[xMax][y].y; // Right
        count++;
    }
    if (yMin >= 0) {
        averageHeight += vertices[x][yMin].y; // Top
        count++;
    }
    if (yMax < MAP_SIZE) {
        averageHeight += vertices[x][yMax].y; // Bottom
        count++;
    }
    return averageHeight / count;
}

/**
 * Applies the Diamond-Square algorithm to the provided set of vertices in a recursive way
 * @param vertices The vertices
 * @param h The smoothness
 * @param stepSize The step size
 * @param randMax Maximum random offset
 */
void diamondSquare(glm::vec3 vertices[MAP_SIZE][MAP_SIZE], float h, int stepSize, float randMax) {
    if (stepSize <= 1) return;
    int halfStepSize = stepSize / 2;

    for (int x = stepSize / 2; x < MAP_SIZE - 1; x += stepSize) {
        for (int y = stepSize / 2; y < MAP_SIZE - 1; y += stepSize) {
            vertices[x][y].y = diamondStep(vertices, x, y, halfStepSize) * randomInRange(-randMax, randMax);
        }
    }

    bool offset = false;
    for (int x = 0; x <= MAP_SIZE - 1; x += halfStepSize) {
        offset = !offset;
        for (int y = offset ? halfStepSize : 0; y <= MAP_SIZE - 1; y += stepSize) {
            vertices[x][y].y = squareStep(vertices, x, y, halfStepSize) * randomInRange(-randMax, randMax);
        }
    }

    randMax = randMax * powf(2, h);
    stepSize = stepSize / 2;
    diamondSquare(vertices, h, stepSize, randMax);
}

int main() {
    GLFWwindow *window;
    GLuint vao, vertexBuffer, vertexShader, fragmentShader, program, raycastTexture;

    if (!glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
#endif

    window = glfwCreateWindow(1080, 720, "322COM ProcGen", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to load GLAD" << std::endl;
        return -1;
    }
    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback(window, glfwFramebufferSizeCallback);
    glfwSetKeyCallback(window, glfwKeyCallback);
    glfwSetCursorPosCallback(window, glfwCursorPosCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Lock mouse to window and hide cursor

    glm::vec3 vertices[MAP_SIZE][MAP_SIZE];
    float randHeight = static_cast<float>(rand() % 10) / 10.f;
    std::cout << "Starting height: " << randHeight << std::endl;

    vertices[0][0].y = randHeight;
    vertices[0][MAP_SIZE - 1].y = randHeight;
    vertices[MAP_SIZE - 1][MAP_SIZE - 1].y = randHeight;
    vertices[MAP_SIZE - 1][0].y = randHeight;
    diamondSquare(vertices, 1.f, MAP_SIZE - 1, 1.f);

    return 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
}