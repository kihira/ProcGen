
#ifndef PROCGEN_FLAT2DARRAY_H
#define PROCGEN_FLAT2DARRAY_H


#include <vec3.hpp>
#include <glad/glad.h>

/**
 * Effectively a 1D array that is accessible as a 2D one. Good for quickly uploaded data to OpenGL
 */
class Mesh {
private:
    GLuint vao; // Vertex array
    GLuint vbo; // Vertices data
    GLuint ibo; // Indices data
    GLenum mode;
    GLsizei elementCount;
    glm::vec3 *data;
    int width, height;
public:
    Mesh(int width, int height);

    glm::vec3 &getValue(int x, int y);

    void setValue(int x, int y, glm::vec3 value);

    glm::vec3 *getData();

    void render();
};


#endif //PROCGEN_FLAT2DARRAY_H
