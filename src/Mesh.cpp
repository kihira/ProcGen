
#include <ext/matrix_transform.hpp>
#include <iostream>
#include "Mesh.h"

#define TRIANGLE_STRIP

Mesh::Mesh(unsigned short width, unsigned short height, Material &material) : width(width), height(height), material(material) {
    data = new Vertex[width * height];

    // Generate initial data
    for (int x = 0; x < width; ++x) {
        for (int z = 0; z < height; ++z) {
            getValue(x, z).position.x = x;
            getValue(x, z).position.y = 0.f;
            getValue(x, z).position.z = z;
            getValue(x, z).normal.x = 0.f;
            getValue(x, z).normal.y = 0.f;
            getValue(x, z).normal.z = 0.f;
        }
    }

    position = glm::vec3(0.f);
    rotation = glm::vec3(0.f);
    scale = glm::vec3(1.f);
    updateModelMatrix();
}

Vertex &Mesh::getValue(int x, int y) {
    return data[width * y + x];
}

Vertex *Mesh::getData() {
    return data;
}

void Mesh::render(Shader *shader) {
    shader->setUniform("model", modelMatrix);
    shader->setUniform("normalMat", glm::transpose(glm::inverse(glm::mat3(modelMatrix))));
    shader->setMaterial(material);

    glBindVertexArray(vao);
    glDrawElements(mode, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_SHORT, nullptr);
}

void Mesh::buildBuffers() {
#ifdef TRIANGLE_STRIP
    // Generate triangle strip indices
    mode = GL_TRIANGLE_STRIP;
    for (unsigned short y = 0; y < height - 1; ++y) {
        for (unsigned short x = 0; x < width; ++x) {
            indices.push_back((y * height) + x);
            indices.push_back((y * height) + x + height);
        }
        // Degenerate triangles
        indices.push_back((y * height) + width + height - (unsigned short) 1);
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

    // Generate normals
    // Optimisations done:
    // - Store normal in vertex and just modify that instead of an external loop
    // todo could optimise this by putting it into the indices loop

    for (int i = 0; i < indices.size() - 2; ++i) {
        auto vert1 = data[indices[i]].position;
        auto vert2 = data[indices[i + 1]].position;
        auto vert3 = data[indices[i + 2]].position;
        data[indices[i]].normal += glm::cross(vert1 - vert2, vert1 - vert3);
        data[indices[i + 1]].normal += glm::cross(vert2 - vert3, vert2 - vert1);
        data[indices[i + 2]].normal += glm::cross(vert3 - vert1, vert3 - vert2);
    }
    for (int vertex = 0; vertex < getSize(); ++vertex) {
        data[vertex].normal = glm::normalize(data[vertex].normal);
    }

    // Generate VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Vertex data
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, getSize() * sizeof(Vertex), getData(), GL_STATIC_DRAW);
    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    glEnableVertexAttribArray(0);
    // Normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)sizeof(glm::vec3));
    glEnableVertexAttribArray(1);

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
