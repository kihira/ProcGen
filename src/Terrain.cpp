
#include <ext/matrix_transform.hpp>
#include <iostream>
#include "Terrain.h"

#define TEX_SCALE .75f

std::default_random_engine Terrain::generator;

Terrain::Terrain(unsigned short size, float maxRand, float h, Shader *shader, Material &material)
        : size(size), maxRand(maxRand), h(h), shader(shader), material(material) {
    data = new Vertex[size * size];

    // Generate initial data
    for (int x = 0; x < size; ++x) {
        for (int z = 0; z < size; ++z) {
            getValue(x, z).position.x = x;
            getValue(x, z).position.y = 0.f;
            getValue(x, z).position.z = z;
            getValue(x, z).normal.x = 0.f;
            getValue(x, z).normal.y = 0.f;
            getValue(x, z).normal.z = 0.f;
        }
    }

    // Generate terrain
    std::uniform_real_distribution<float> distribution(-maxRand, maxRand);
    getValue(0, 0).position.y = distribution(generator);
    getValue(0, size - 1).position.y = distribution(generator);
    getValue(size - 1, size - 1).position.y = distribution(generator);
    getValue(size - 1, 0).position.y = distribution(generator);

    diamondSquare(size - 1, maxRand);

    buildBuffers();

    // Set world transform
    position = glm::vec3(0.f);
    rotation = glm::vec3(0.f);
    scale = glm::vec3(1.f);
    updateModelMatrix();
}

Vertex &Terrain::getValue(int x, int y) {
    assert(x < size);
    assert(y < size);

    return data[size * y + x];
}

Vertex *Terrain::getData() {
    return data;
}

void Terrain::render() {
    shader->use();
    shader->setUniform("model", modelMatrix);
    shader->setUniform("normalMat", glm::transpose(glm::inverse(glm::mat3(modelMatrix))));
    shader->setMaterial(material);
    shader->setUniform("minY", minY);
    shader->setUniform("maxY", maxY);

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
    // Generate triangle strip indices
    mode = GL_TRIANGLE_STRIP;
    for (unsigned short y = 0; y < size - 1; ++y) {
        for (unsigned short x = 0; x < size; ++x) {
            indices.push_back((y * size) + x);
            indices.push_back((y * size) + x + size);
        }
        // Degenerate triangles
        indices.push_back((y * size) + size + size - (unsigned short) 1);
        indices.push_back((y * size) + size);
    }

    // Remove last two which are degenerates
    indices.pop_back();
    indices.pop_back();

    // Generate UVs
    float uScale = static_cast<float>(size) * TEX_SCALE;
    float vScale = static_cast<float>(size) * TEX_SCALE;
    for (int x = 0; x < size; ++x) {
        for (int y = 0; y < size; ++y) {
            // Calculate min and max y
            if (getValue(x, y).position.y > maxY) {
                maxY = getValue(x, y).position.y;
            } else if (getValue(x, y).position.y < minY) {
                minY = getValue(x, y).position.y;
            }

            getValue(x, y).uv.x = uScale * (static_cast<float>(x) / static_cast<float>(size - 1));
            getValue(x, y).uv.y = vScale * (static_cast<float>(y) / static_cast<float>(size - 1));
        }
    }

    // Generate normals

    // Normals per quad (which is made of two triangles)
    glm::vec3 quadNormals[size - 1][size - 1][2];

    // Calculate the normals per quad (and their two triangles)
    for (int x = 0; x < size - 1; ++x) {
        for (int y = 0; y < size - 1; ++y) {
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
    for (int x = 0; x < size; ++x) {
        for (int y = 0; y < size; ++y) {
            glm::vec3 normal(0.f);
            if (y > 0) {
                // Get top left quad, second triangle
                if (x > 0) {
                    normal += quadNormals[x-1][y-1][1];
                }
                // Top right quad, both triangles
                if (x < size - 1) {
                    normal += quadNormals[x][y-1][0];
                    normal += quadNormals[x][y-1][1];
                }
            }
            if (y < size - 1) {
                // Bottom left quad, both triangles
                if (x > 0) {
                    normal += quadNormals[x-1][y][0];
                    normal += quadNormals[x-1][y][1];
                }
                // Bottom right quad, first triangle
                if (x < size - 1) {
                    normal += quadNormals[x][y][0];
                }
            }

            // Set actual normal
            normal = glm::normalize(normal);
            getValue(x, y).normal = normal;
        }
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
    return size * size;
}

void Terrain::updateModelMatrix() {
    modelMatrix = glm::translate(glm::mat4(1.f), position);
    modelMatrix = glm::rotate(modelMatrix, rotation.x, glm::vec3(1.f, 0.f, 0.f));
    modelMatrix = glm::rotate(modelMatrix, rotation.y, glm::vec3(0.f, 1.f, 0.f));
    modelMatrix = glm::rotate(modelMatrix, rotation.z, glm::vec3(0.f, 0.f, 1.f));
    // modelMatrix = glm::scale(modelMatrix, scale);
}

float Terrain::diamondStep(int x, int y, int stepSize) {
    float averagesize = 0.f;
    int xMin = x - stepSize;
    int xMax = x + stepSize;
    int yMin = y - stepSize;
    int yMax = y + stepSize;
    averagesize += getValue(xMin, yMin).position.y; // Top left
    averagesize += getValue(xMin, yMax).position.y; // Bottom left
    averagesize += getValue(xMax, yMin).position.y; // Top right
    averagesize += getValue(xMax, yMax).position.y; // Bottom right
    return averagesize / 4.f;
}

/**
 * Calculates the average size for the provided vertex based on a diamond pattern around it
 * @param vertices
 * @param x
 * @param y
 * @param stepSize
 */
float Terrain::squareStep(int x, int y, int stepSize) {
    float averagesize = 0.f;
    int xMin = x - stepSize;
    int xMax = x + stepSize;
    int yMin = y - stepSize;
    int yMax = y + stepSize;
    if (xMin < 0) {
        xMin = size - abs(xMin);
    }
    averagesize += getValue(xMin, y).position.y; // Left
    if (xMax >= size) {
        xMax = xMax - size;
    }
    averagesize += getValue(xMax, y).position.y; // Right
    if (yMin < 0) {
        yMin = size - abs(yMin);
    }
    averagesize += getValue(x, yMin).position.y; // Top
    if (yMax >= size) {
        yMax = yMax - size;
    }
    averagesize += getValue(x, yMax).position.y; // Bottom
    return averagesize / 4.f;
}

/**
 * Applies the Diamond-Square algorithm to the provided set of vertices in a recursive way
 * @param mesh The mesh data
 * @param h The smoothness
 * @param stepSize The step size
 * @param randMax Maximum random offset
 */
void Terrain::diamondSquare(int stepSize, float randMax) {
    if (stepSize <= 1) return;
    int halfStepSize = stepSize / 2;
    std::uniform_real_distribution<float> distribution(-randMax, randMax);

    for (int x = halfStepSize; x < size - 1; x += stepSize) {
        for (int y = halfStepSize; y < size - 1; y += stepSize) {
            getValue(x, y).position.y = diamondStep(x, y, halfStepSize) + distribution(generator);
        }
    }

    bool offset = false;
    for (int x = 0; x <= size - 1; x += halfStepSize) {
        offset = !offset;
        for (int y = offset ? halfStepSize : 0; y <= size - 1; y += stepSize) {
            getValue(x, y).position.y = squareStep(x, y, halfStepSize) + distribution(generator);
        }
    }

    randMax = randMax * powf(2, -h);
    stepSize = halfStepSize;
    diamondSquare(stepSize, randMax);
}

void Terrain::setPosition(const glm::vec3 &position) {
    Terrain::position = position;
    updateModelMatrix();
}
