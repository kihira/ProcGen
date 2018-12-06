
#include "Tree.h"
#include <random>
#include <geometric.hpp>
#include <ext/matrix_transform.hpp>
#include <iostream>

Tree::Tree(TreeSettings &settings, glm::vec3 origin, Shader *shader) : settings(settings), position(origin), shader(shader) {

    // Generate attraction points
    glm::vec3 crownSizeHalf = settings.crownSize / 2.f;
    std::default_random_engine generator;
    std::uniform_real_distribution<float> xDist(-crownSizeHalf.x, crownSizeHalf.x);
    std::uniform_real_distribution<float> yDist(-crownSizeHalf.y, crownSizeHalf.y);
    std::uniform_real_distribution<float> zDist(-crownSizeHalf.z, crownSizeHalf.z);

    for (int i = 0; i < settings.attractionPoints; ++i) {
        auto pos = glm::vec3(settings.crownCentre);
        pos.x += xDist(generator);
        pos.y += yDist(generator);
        pos.z += zDist(generator);

        AttractionPoint point{};
        point.position = pos;
        attractionPoints.push_back(point);
    }

    // Create root node
    auto rootNode = new Node();
    rootNode->position = position;
    rootNode->direction = glm::vec3(0.f, 1.f, 0.f);
    nodes.push_back(rootNode);

    while (!attractionPoints.empty()) {
        grow();
    }
    buildBuffers();
}

void Tree::grow() {
    if (attractionPoints.empty()) return;

    for (auto point = attractionPoints.begin(); point != attractionPoints.end(); ++point) {
        point->closestNode = nullptr;

        for (auto node : nodes) {
            auto distance = glm::distance(point->position, node->position);
            if (distance < settings.killDistance) {
                // Remove node as we've now reached it
                attractionPoints.erase(point);
                continue;
            }
            else if (distance < settings.influenceRadius) {
                // Check if we're now the closest point and if so, set it
                if (point->closestNode == nullptr || distance < glm::distance(point->closestNode->position, point->position)) {
                    point->closestNode = node;
                }
            }
        }

        // Move node towards point
        if (point->closestNode != nullptr) {
            auto direction = point->position - point->closestNode->position;
            direction = glm::normalize(direction);
            point->closestNode->direction += direction;
            point->closestNode->influenceCount += 1;
        }
    }

    // Generate new nodes
    std::vector<Node *> newNodes;
    for (auto node : nodes) {
        if (node->influenceCount > 0) {
            auto direction = glm::normalize(node->direction / (float)node->influenceCount);
            auto newNode = new Node();
            newNode->parent = node;
            newNode->direction = direction;
            newNode->position = node->position + direction * settings.nodeSize;
            newNodes.push_back(newNode);
        }
    }

    // Add new nodes
    nodes.insert(nodes.end(), newNodes.begin(), newNodes.end());

}

void Tree::buildBuffers() {
    std::vector<glm::vec3> vertexData;
    for (auto &node : nodes) {
        if (node->parent != nullptr) {
            vertexData.push_back(node->position);
            vertexData.push_back(node->parent->position);
        }
    }

    std::vector<unsigned short> indicesData;
    indicesData.reserve(nodes.size());
    for (int i = 0; i < nodes.size(); ++i) {
        indicesData.push_back(i);
    }

    indices = indicesData.size();

    glLineWidth(3);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size(), &vertexData[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesData.size(), &indicesData[0], GL_STATIC_DRAW);

    model = glm::translate(glm::mat4(1.f), position);
}

void Tree::render() {
    shader->use();
    shader->setUniform("model", model);

    glBindVertexArray(vao);
    glDrawElements(GL_LINES, indices, GL_UNSIGNED_SHORT, nullptr);
}
