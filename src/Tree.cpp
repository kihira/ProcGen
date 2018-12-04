
#include "Tree.h"
#include <random>
#include <geometric.hpp>
#include <ext/matrix_transform.hpp>

Tree::Tree(TreeSettings &settings, glm::vec3 origin, Shader *shader) : position(origin), shader(shader) {

    // Generate attraction points
    attractionPoints.reserve(settings.attractionPoints);
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

    grow();
}

void Tree::grow() {
    if (attractionPoints.empty()) return;

    for (auto &point : attractionPoints) {
        point.closestNode = nullptr;

        for (auto node : nodes) {
            auto distance = glm::distance(point.position, node->position);
            if (distance < settings.killDistance) {
                // Remove node as we've now reached it
                attractionPoints.erase(std::find(attractionPoints.begin(), attractionPoints.end(), point));
            }
            else if (distance < settings.influenceRadius) {
                // Check if we're now the closest point and if so, set it
                if (point.closestNode == nullptr || distance < glm::distance(point.closestNode->position, point.position)) {
                    point.closestNode = node;
                }
            }
        }

        // Move node direction towards point
        if (point.closestNode != nullptr) {
            auto direction = point.position - point.closestNode->position;
            direction = glm::normalize(direction);
            point.closestNode->direction += direction;
            point.closestNode->affectCount++;
        }
    }

    // Generate new nodes
    std::vector<Node *> newNodes;
    for (auto &node : nodes) {
        if (node->affectCount > 0) {
            auto direction = glm::normalize(node->direction / (float)node->affectCount);
            auto newNode = new Node();
            newNode->parent = node;
            newNode->direction = direction;
            newNode->position = node->position + direction * settings.nodeSize;
        }
    }

    // Add new nodes
    nodes.insert(nodes.end(), newNodes.begin(), newNodes.end());

}

void Tree::buildBuffers() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    std::vector<glm::vec3> vertexData;
    for (auto &node : nodes) {
        vertexData.push_back(node->position);
    }
    glBufferData(GL_ARRAY_BUFFER, vertexData.size(), &vertexData[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    std::vector<unsigned short> indicesData;
    indicesData.reserve(nodes.size());
    for (int i = 0; i < nodes.size(); ++i) {
        indicesData.push_back(i);
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesData.size(), &indicesData[0], GL_STATIC_DRAW);

    model = glm::translate(glm::mat4(1.f), position);
}

void Tree::render() {
    shader->use();
    shader->setUniform("model", model);

    glBindVertexArray(vao);
    glDrawElements(GL_POINTS, indices, GL_UNSIGNED_SHORT, nullptr);
}
