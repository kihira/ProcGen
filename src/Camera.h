#ifndef PROCGEN_CAMERA_H
#define PROCGEN_CAMERA_H


#include <glm.hpp>

class Camera {
private:
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
    float near_ = .1f;
    float far_ = 1000.f;
    float translateSpeed = .1f;
    float rotationSpeed = .25f;
public:
    void updateProjectionMatrix(int width, int height);

    void updateViewMatrix();

    void handleCursorMove(double xPos, double yPos);

    void handleKey(int key, int scancode, int action, int mods);

    const glm::mat4 &getViewMatrix() const;

    const glm::mat4 &getProjMatrix() const;
};


#endif //PROCGEN_CAMERA_H
