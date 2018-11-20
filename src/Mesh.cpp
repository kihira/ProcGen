
#include "Mesh.h"

Mesh::Mesh(unsigned short width, unsigned short height) : width(width), height(height) {
    data = new glm::vec3[width * height];
    mode = GL_TRIANGLE_STRIP;
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

void Mesh::render() {
    glBindVertexArray(vao);
    glDrawElements(mode, elementCount, GL_UNSIGNED_INT, nullptr);
}

void Mesh::buildBuffers() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Vertex data
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, getSize() * sizeof(glm::vec3), getData(), GL_STATIC_DRAW);

    // Generate indices
    for (unsigned short y = 0; y < height; y++) {
        for (unsigned short x = 0; x <= width; x++) {
            indices.push_back((y * height) + x);
            indices.push_back((y * height) + x + height);
        }
        // Degenerate triangles
        indices.push_back((y * height) + width);
        indices.push_back((y * height) + width + height);
    }

    // Indices data
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
}

unsigned int Mesh::getSize() {
    return width * height;
}
