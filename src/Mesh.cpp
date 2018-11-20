
#include "Mesh.h"

Mesh::Mesh(int width, int height) : width(width), height(height) {
    data = new glm::vec3[width * height];
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
