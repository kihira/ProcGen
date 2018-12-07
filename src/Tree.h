
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
    unsigned int branchSides; // Sides to the cylinder for trees
    float branchThickness;
};

// Branches
struct Node {
    Node *parent;
    glm::vec3 position;
    glm::vec3 originalDirection;
    glm::vec3 influenceDirection;
    int influenceCount = 0;

    Node(Node *parent, glm::vec3 position, glm::vec3 direction) :
        parent(parent), position(position), originalDirection(direction), influenceDirection(direction) {}

    void resetInfluence() {
        influenceDirection = originalDirection;
        influenceCount = 0;
    }
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

    std::vector<glm::vec3> generateCylinderVertices();

    void buildBuffers();
public:
    Tree(TreeSettings &settings, glm::vec3 origin, Shader *shader);

    void render();
};


#endif //PROCGEN_TREE_H
