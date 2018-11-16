
#include "mesh.h"

void Mesh::render() {
    glBindVertexArray(vao);
    glDrawElements(mode, elementCount, GL_UNSIGNED_INT, nullptr);
}
