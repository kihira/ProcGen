
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
};

class Tree {
private:
    glm::vec3 position;
public:
    Tree(TreeSettings &crownSettings, glm::vec3 origin);

    void render();
};


#endif //PROCGEN_TREE_H
