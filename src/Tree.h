
#ifndef PROCGEN_TREE_H
#define PROCGEN_TREE_H


#include <vec3.hpp>
#include <vector>

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
    int affectCount;
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
    std::vector<AttractionPoint> attractionPoints;
    std::vector<Node *> nodes;

    void grow();
public:
    Tree(TreeSettings &crownSettings, glm::vec3 origin);

    void render();
};


#endif //PROCGEN_TREE_H
