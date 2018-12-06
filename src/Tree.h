
#ifndef PROCGEN_TREE_H
#define PROCGEN_TREE_H


#include <vec3.hpp>
#include <vector>
#include <list>
#include "Shader.h"

struct TreeSettings {
    glm::vec3 crownCentre;
    glm::vec3 crownSize;
    unsigned int attractionPoints;
    float influenceRadius; // di
    float killDistance; // dk
    float nodeSize;
};

// Branches
struct Node {
    Node *parent;
    glm::vec3 position;
    glm::vec3 direction;
    int influenceCount;
};

// "leaves"
struct AttractionPoint {
    glm::vec3 position;
    Node *closestNode;

};

class Tree {
private:
    glm::vec3 position;

    TreeSettings settings;
    std::list<AttractionPoint> attractionPoints;
    std::vector<Node *> nodes;

    // Render
    Shader *shader;
    GLuint vao;
    GLuint ibo;
    GLuint vbo;
    GLuint indices;
    glm::mat4 model;

    void grow();

    void buildBuffers();
public:
    Tree(TreeSettings &settings, glm::vec3 origin, Shader *shader);

    void render();
};


#endif //PROCGEN_TREE_H
