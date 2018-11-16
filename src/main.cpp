#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

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

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
}