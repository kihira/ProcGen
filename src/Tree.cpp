
#include "Tree.h"
#include <random>

// Branches
struct Node {
    Node *parent;
};

// "leaves"
struct AttractionPoint {
    glm::vec3 position;
    Node *closestNode;

};

Tree::Tree(TreeSettings &settings, glm::vec3 origin) : position(origin) {
    std::vector<glm::vec3> attractionPoints;
    attractionPoints.reserve(settings.attractionPoints);

    // Generate attraction points
    glm::vec3 crownSizeHalf = settings.crownSize / 2.f;
    std::default_random_engine generator;
    std::uniform_real_distribution<float> xDist(-crownSizeHalf.x, crownSizeHalf.x);
    std::uniform_real_distribution<float> yDist(-crownSizeHalf.y, crownSizeHalf.y);
    std::uniform_real_distribution<float> zDist(-crownSizeHalf.z, crownSizeHalf.z);
    for (int i = 0; i < settings.attractionPoints; ++i) {
        auto leaf = glm::vec3(settings.crownCentre);
        leaf.x = xDist(generator);
        leaf.y = yDist(generator);
        leaf.z = zDist(generator);
    }
}

void Tree::render() {

}
