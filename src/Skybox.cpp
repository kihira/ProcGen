
#include <string>
#include <stb_image.h>
#include <iostream>
#include "Skybox.h"

Skybox::Skybox(Shader *shader, std::string texBasePath) : shader(shader) {
    // Load skybox texture
    int width, height, channels;
    unsigned char *imageData;

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    for (int i = 0; i < 6; ++i) {
        imageData = stbi_load((texBasePath + texNames[i] + ".png").c_str(), &width, &height, &channels, 4);
        if (imageData == nullptr) {
            std::cerr << "Failed to load skybox file: " << texBasePath << std::endl;
            return;
        }

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
        stbi_image_free(imageData);
    }

    // Load cube
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
}

void Skybox::render(Camera &camera) {
    glDisable(GL_DEPTH_TEST);

    // By creating a mat3, we drop the positional data but keep rotation so it always follows the camera
    auto view = glm::mat4(glm::mat3(camera.getViewMatrix()));

    shader->use();
    shader->setUniform("projection", camera.getProjMatrix());
    shader->setUniform("view", view);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, sizeof(cubeIndices), GL_UNSIGNED_SHORT, nullptr);

    glEnable(GL_DEPTH_TEST);
}
