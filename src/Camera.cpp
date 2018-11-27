#include <ext.hpp>
#include <GLFW/glfw3.h>
#include "Camera.h"

void Camera::updateProjectionMatrix(const int width, const int height) {
    projMatrix = glm::perspective(glm::radians(fov), (float) width / (float) height, near_, far_);
    // todo shader->setUniform("projection", projMatrix);
}

void Camera::updateViewMatrix() {
    // Prevent "flipping" the camera when looking up and down
    pitch = glm::clamp(pitch, -89.9f, 89.9f);

    // Calculate looking direction
    direction.x = cosf(glm::radians(yaw)) * cosf(glm::radians(pitch));
    direction.y = sinf(glm::radians(-pitch));
    direction.z = sinf(glm::radians(yaw)) * cosf(glm::radians(pitch));
    direction = glm::normalize(direction);

    viewMatrix = glm::lookAt(position, position + direction, glm::vec3(0.f, 1.f, 0.f));
}

void Camera::handleCursorMove(double xPos, double yPos) {
    auto cursorDelta = glm::dvec2(xPos - cursorPosLastX, yPos - cursorPosLastY);

    yaw += cursorDelta.x * rotationSpeed;
    pitch += cursorDelta.y * rotationSpeed;
    updateViewMatrix();

    cursorPosLastX = xPos;
    cursorPosLastY = yPos;
}

void Camera::handleKey(int key, int scancode, int action, int mods) {
    // Repeating key used for camera controls
    if (action == GLFW_REPEAT || action == GLFW_KEY_DOWN) {
        switch (key) {
            case GLFW_KEY_W:
                position += direction * translateSpeed;
                break;
            case GLFW_KEY_S:
                position += -direction * translateSpeed;
                break;
            case GLFW_KEY_A:
                position += glm::cross(glm::vec3(0.f, 1.f, 0.f), direction) * translateSpeed;
                break;
            case GLFW_KEY_D:
                position += -glm::cross(glm::vec3(0.f, 1.f, 0.f), direction) * translateSpeed;
                break;
            case GLFW_KEY_R:
                position.y += translateSpeed;
                break;
            case GLFW_KEY_F:
                position.t -= translateSpeed;
                break;
//            case GLFW_KEY_Q:
//                direction.z -= rotationSpeed;
//                break;
//            case GLFW_KEY_E:
//                direction.z += rotationSpeed;
//                break;
        }
        updateViewMatrix();
    }
}

const glm::mat4 &Camera::getViewMatrix() const {
    return viewMatrix;
}

const glm::mat4 &Camera::getProjMatrix() const {
    return projMatrix;
}
