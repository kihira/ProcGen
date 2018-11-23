
#include <ext/matrix_transform.hpp>
#include <iostream>
#include "Mesh.h"

#define TRIANGLE_STRIP

Mesh::Mesh(unsigned short width, unsigned short height, Material &material) : width(width), height(height), material(material) {
    data = new glm::vec3[width * height];

    // Generate xz coords
    for (int x = 0; x < width; ++x) {
        for (int z = 0; z < height; ++z) {
            getValue(x, z).x = x;
            getValue(x, z).y = 0.f;
            getValue(x, z).z = z;
        }
    }

    position = glm::vec3(0.f);
    rotation = glm::vec3(0.f);
    scale = glm::vec3(1.f);
    updateModelMatrix();
}

glm::vec3 &Mesh::getValue(int x, int y) {
    return data[width * y + x];
}

void Mesh::setValue(int x, int y, glm::vec3 value) {
    data[width * y + x] = value;
}

glm::vec3 *Mesh::getData() {
    return data;
}

void Mesh::render(Shader *shader) {
    shader->setUniform("model", modelMatrix);
    shader->setMaterial(material);

    glBindVertexArray(vao);
    glDrawElements(mode, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_SHORT, nullptr);
}

void Mesh::buildBuffers() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Vertex data
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, getSize() * sizeof(glm::vec3), getData(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

#ifdef TRIANGLE_STRIP
    // Generate triangle strip indices
    mode = GL_TRIANGLE_STRIP;
    for (unsigned short y = 0; y < height - 1; ++y) {
        for (unsigned short x = 0; x < width; ++x) {
            indices.push_back((y * height) + x);
            indices.push_back((y * height) + x + height);
        }
        // Degenerate triangles
        indices.push_back((y * height) + width + height - (ushort) 1);
        indices.push_back((y * height) + width);
    }

    // Remove last two which are degenerates
    indices.pop_back();
    indices.pop_back();
#else
    // Generate triangle indices
    mode = GL_TRIANGLES;
    for (unsigned short y = 0; y < height - 1; ++y) {
        for (unsigned short x = 0; x < width - 1; ++x) {
            indices.push_back((y * height) + x);
            indices.push_back((y * height) + x + height);
            indices.push_back((y * height) + x + static_cast<unsigned short>(1));

            indices.push_back((y * height) + x + static_cast<unsigned short>(1));
            indices.push_back((y * height) + x + height);
            indices.push_back((y * height) + x + height + static_cast<unsigned short>(1));
        }
    }
#endif

    // Indices data
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
}

unsigned int Mesh::getSize() {
    return width * height;
}

void Mesh::updateModelMatrix() {
    modelMatrix = glm::translate(glm::mat4(1.f), position);
    modelMatrix = glm::rotate(modelMatrix, rotation.x, glm::vec3(1.f, 0.f, 0.f));
    modelMatrix = glm::rotate(modelMatrix, rotation.y, glm::vec3(0.f, 1.f, 0.f));
    modelMatrix = glm::rotate(modelMatrix, rotation.z, glm::vec3(0.f, 0.f, 1.f));
    // modelMatrix = glm::scale(modelMatrix, scale);
}
