
#ifndef PROCGEN_FLAT2DARRAY_H
#define PROCGEN_FLAT2DARRAY_H


#include <vec3.hpp>
#include <glad/glad.h>
#include <vector>
#include <random>
#include "Shader.h"

struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 emmisive;
    float shininess;
    std::vector<GLuint> textures;
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

/**
 * Effectively a 1D array that is accessible as a 2D one. Good for quickly uploaded data to OpenGL
 */
class Terrain {
private:
    static std::default_random_engine generator;

    GLuint vao; // Vertex array
    GLuint vbo; // Vertices data
    GLuint ibo; // Indices data
    GLenum mode;
    Material material;
    Shader *shader;
    std::vector<unsigned short> indices;

    // World space data
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    glm::mat4 modelMatrix;

    // Data for generating terrain
    unsigned short size;
    float minY, maxY;
    float maxRand, h;
    Vertex *data;

    float diamondStep(int x, int y, int stepSize);

    float squareStep(int x, int y, int stepSize);

    void diamondSquare(int stepSize, float randMax);
public:
    Terrain(unsigned short size, float maxRand, float h, Shader *shader, Material &material);

    Vertex &getValue(int x, int y);

    Vertex *getData();

    unsigned int getSize();

    /**
     * Renders the current mesh
     */
    void render();

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
