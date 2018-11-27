
#include <ext/matrix_transform.hpp>
#include <iostream>
#include "Terrain.h"

#define TRIANGLE_STRIP
#define TEX_SCALE .75f

Terrain::Terrain(unsigned short width, unsigned short height, Material &material) : width(width), height(height), material(material) {
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

Vertex &Terrain::getValue(int x, int y) {
    assert(x < width);
    assert(y < height);

    return data[width * y + x];
}

Vertex *Terrain::getData() {
    return data;
}

void Terrain::render(Shader *shader) {
    shader->setUniform("model", modelMatrix);
    shader->setUniform("normalMat", glm::transpose(glm::inverse(glm::mat3(modelMatrix))));
    shader->setMaterial(material);

    // Bind textures
    for (int i = 0; i < material.textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, material.textures[i]);
        shader->setUniform(("textures["+ std::to_string(i) + "]").c_str(), i);
    }

    glBindVertexArray(vao);
    glDrawElements(mode, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_SHORT, nullptr);
}

void Terrain::buildBuffers() {
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

    // Generate UVs
    float uScale = static_cast<float>(width) * TEX_SCALE;
    float vScale = static_cast<float>(height) * TEX_SCALE;
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            getValue(x, y).uv.x = uScale * (static_cast<float>(x) / static_cast<float>(width - 1));
            getValue(x, y).uv.y = vScale * (static_cast<float>(y) / static_cast<float>(height - 1));
        }
    }

    // Generate normals

    // Normals per quad (which is made of two triangles)
    glm::vec3 quadNormals[width - 1][height - 1][2];

    // Calculate the normals per quad (and their two triangles)
    for (int x = 0; x < width - 1; ++x) {
        for (int y = 0; y < height - 1; ++y) {
            // First triangle
            auto vert1 = getValue(x, y).position;
            auto vert2 = getValue(x, y+1).position;
            auto vert3 = getValue(x+1, y).position;

            auto normal = glm::cross(vert1 - vert2, vert1 - vert3);
            normal += glm::cross(vert2 - vert3, vert2 - vert1);
            normal += glm::cross(vert3 - vert1, vert3 - vert2);

            quadNormals[x][y][0] = normal;

            // Second triangle
            vert1 = getValue(x+1, y+1).position;
            vert2 = getValue(x+1, y).position;
            vert3 = getValue(x, y+1).position;

            normal = glm::cross(vert1 - vert2, vert1 - vert3);
            normal += glm::cross(vert2 - vert3, vert2 - vert1);
            normal += glm::cross(vert3 - vert1, vert3 - vert2);

            quadNormals[x][y][1] = normal;
        }
    }

    // Calculate normal per vertex based upon previous calculation
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            glm::vec3 normal(0.f);
            if (y > 0) {
                // Get top left quad, second triangle
                if (x > 0) {
                    normal += quadNormals[x-1][y-1][1];
                }
                // Top right quad, both triangles
                if (x < width - 1) {
                    normal += quadNormals[x][y-1][0];
                    normal += quadNormals[x][y-1][1];
                }
            }
            if (y < height - 1) {
                // Bottom left quad, both triangles
                if (x > 0) {
                    normal += quadNormals[x-1][y][0];
                    normal += quadNormals[x-1][y][1];
                }
                // Bottom right quad, first triangle
                if (x < width - 1) {
                    normal += quadNormals[x][y][0];
                }
            }

            // Set actual normal
            normal = glm::normalize(normal);
            getValue(x, y).normal = normal;
        }
    }

//    for (int i = 0; i < indices.size() - 2; ++i) {
//        auto vert1 = data[indices[i]].position;
//        auto vert2 = data[indices[i + 1]].position;
//        auto vert3 = data[indices[i + 2]].position;
//        data[indices[i]].normal += glm::cross(vert1 - vert2, vert1 - vert3);
//        data[indices[i + 1]].normal += glm::cross(vert2 - vert3, vert2 - vert1);
//        data[indices[i + 2]].normal += glm::cross(vert3 - vert1, vert3 - vert2);
//    }
//    for (int vertex = 0; vertex < getSize(); ++vertex) {
//        data[vertex].normal = glm::normalize(data[vertex].normal);
//    }

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
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)sizeof(Vertex::position));
    glEnableVertexAttribArray(1);
    // UV
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(sizeof(Vertex::position) + sizeof(Vertex::normal)));
    glEnableVertexAttribArray(2);

    // Indices data
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
}

unsigned int Terrain::getSize() {
    return width * height;
}

void Terrain::updateModelMatrix() {
    modelMatrix = glm::translate(glm::mat4(1.f), position);
    modelMatrix = glm::rotate(modelMatrix, rotation.x, glm::vec3(1.f, 0.f, 0.f));
    modelMatrix = glm::rotate(modelMatrix, rotation.y, glm::vec3(0.f, 1.f, 0.f));
    modelMatrix = glm::rotate(modelMatrix, rotation.z, glm::vec3(0.f, 0.f, 1.f));
    // modelMatrix = glm::scale(modelMatrix, scale);
}
