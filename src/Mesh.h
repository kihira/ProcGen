
#ifndef PROCGEN_FLAT2DARRAY_H
#define PROCGEN_FLAT2DARRAY_H


#include <vec3.hpp>
#include <glad/glad.h>
#include <vector>
#include "Shader.h"

/**
 * Effectively a 1D array that is accessible as a 2D one. Good for quickly uploaded data to OpenGL
 */
class Mesh {
private:
    GLuint vao; // Vertex array
    GLuint vbo; // Vertices data
    GLuint ibo; // Indices data
    GLenum mode;
    std::vector<unsigned short> indices;

    // World space data
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    glm::mat4 modelMatrix;

    // Data for generating terrain
    unsigned short width, height;
    glm::vec3 *data;
public:
    Mesh(unsigned short  width, unsigned short  height);

    glm::vec3 &getValue(int x, int y);

    void setValue(int x, int y, glm::vec3 value);

    glm::vec3 *getData();

    unsigned int getSize();

    /**
     * Renders the current mesh
     */
    void render(Shader *shader);

    /**
     * Generates the buffers and fills them with the mesh data
     */
    void buildBuffers();

    /**
     * Updates the model matrix.
     * This should ALWAYS be called after updating position/rotation/scale
     */
    void updateModelMatrix();
};


#endif //PROCGEN_FLAT2DARRAY_H
