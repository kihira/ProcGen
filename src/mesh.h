
#ifndef PROCGEN_MESH_H
#define PROCGEN_MESH_H


#include <glad/glad.h>

class Mesh {
private:
    GLuint vao; // Vertex array
    GLuint vbo; // Vertices data
    GLuint ibo; // Indices data
    GLenum mode;
    GLsizei elementCount;
public:
    void render();
};


#endif //PROCGEN_MESH_H
