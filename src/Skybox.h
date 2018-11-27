#ifndef PROCGEN_SKYBOX_H
#define PROCGEN_SKYBOX_H


#include <glad/glad.h>
#include "Shader.h"
#include "Camera.h"

class Skybox {
private:
    GLuint textureId;
    GLuint vao;
    GLuint ibo;
    GLuint vbo;
    Shader *shader;
public:
    Skybox(Shader *shader, std::string texBasePath);

    void render(Camera &camera);
};

static const std::string texNames[6] {
        "right",
        "left",
        "top",
        "bottom",
        "front",
        "back"
};

static const GLfloat cubeVerts[] = {
        -1, -1, 1,
        1, -1, 1,
        1, 1, 1,
        -1, 1, 1,
        -1, -1, -1,
        1, -1, -1,
        1, 1, -1,
        -1, 1, -1
};

static const GLushort cubeIndices[] = {
        0, 1, 2, // BACK
        2, 3, 0,
        1, 5, 6, // RIGHT
        6, 2, 1,
        7, 6, 5, // FRONT
        5, 4, 7,
        4, 0, 3, // LEFT
        3, 7, 4,
        4, 5, 1, // BOTTOM
        1, 0, 4,
        3, 2, 6, // TOP
        6, 7, 3
};


#endif //PROCGEN_SKYBOX_H
