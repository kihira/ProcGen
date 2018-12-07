
#include "Tree.h"
#include <random>
#include <geometric.hpp>
#include <ext/matrix_transform.hpp>
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/rotate_vector.hpp>

#define CYLINDERS

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
    auto rootNode = new Node(nullptr, position, glm::vec3(0.f, 1.f, 0.f));
    nodes.push_back(rootNode);

    unsigned short iterations = 0;
    while (!attractionPoints.empty() && iterations < settings.maxIterations) {
        grow();
        iterations++;
    }

    buildBuffers();
}

void Tree::grow() {
    if (attractionPoints.empty()) return;

    // Loop over attraction points and see if any nodes are attracted
    for (auto &attractionPoint : attractionPoints) {
        attractionPoint.closestNode = nullptr;

        for (auto node : nodes) {
            auto distance = glm::distance(attractionPoint.position, node->position);
            if (distance > (settings.killDistance * settings.nodeSize) && distance < (settings.influenceRadius * settings.nodeSize)) {
                // Check if we're now the closest point and if so, set it
                if (attractionPoint.closestNode == nullptr || distance < glm::distance(attractionPoint.closestNode->position,
                                                                                       attractionPoint.position)) {
                    attractionPoint.closestNode = node;
                }
            }
        }

        // Move node towards point
        if (attractionPoint.closestNode != nullptr) {
            auto direction = attractionPoint.position - attractionPoint.closestNode->position;
            direction = glm::normalize(direction);
            attractionPoint.closestNode->influenceDirection += direction;
            attractionPoint.closestNode->influenceCount += 1;
        }
    }

    // Generate new nodes
    std::vector<Node *> newNodes;
    for (auto node : nodes) {
        if (node->influenceCount > 0) {
            // Calculate new position and direction of branch based on point influence
            auto direction = glm::normalize(node->influenceDirection / (float)node->influenceCount);
            auto newNode = new Node(node, node->position + direction * settings.nodeSize, direction);
            newNodes.push_back(newNode);

            // Reset node direction/influence
            node->resetInfluence();
        }
    }

    // Add new nodes
    nodes.insert(nodes.end(), newNodes.begin(), newNodes.end());

    // Remove attraction points
    // todo see if we can collapse this back into the first loop
    for (auto node : nodes) {
        auto point = attractionPoints.begin();
        while (point != attractionPoints.end()) {
            auto distance = glm::distance(point->position, node->position);
            if (distance <= (settings.killDistance * settings.nodeSize)) {
                // Remove node as we've now reached it
                point = attractionPoints.erase(point);
            } else {
                point++;
            }
        }
    }

}

std::vector<glm::vec3> Tree::generateBranchVertices(Node *node) {
    std::vector<glm::vec3> vertices;
    vertices.reserve(settings.branchSides * 2);
    auto segmentSize = glm::radians(360.f / static_cast<float>(settings.branchSides));
    glm::mat4 bottomTransform;
    glm::mat4 bottomRotation;
    glm::mat4 topTransform;
    glm::mat4 topRotation;
    glm::mat4 transform = glm::lookAt(node->position, node->direction, glm::vec3(0.f, 1.f, 0.f));;

    for (int i = 0; i < settings.branchSides; ++i) {
        auto bottomVertex = glm::vec3(0.f, 0.f, settings.nodeSize * .5f);
        bottomVertex = glm::rotateY(bottomVertex, segmentSize * i);
        bottomVertex = transform * glm::vec4(bottomVertex, 1.f);
        vertices.emplace_back(bottomVertex);

        auto topVertex = glm::vec3(0.f, settings.nodeSize, settings.nodeSize * .5f);
        topVertex = glm::rotateY(topVertex, segmentSize * i);
        topVertex = transform * glm::vec4(topVertex, 1.f);
        vertices.emplace_back(topVertex);
    }

    return vertices;
}

void Tree::buildBuffers() {
    std::vector<glm::vec3> vertexData;
#ifdef CYLINDERS
    glm::mat4 transform(1.f);

    for (auto node : nodes) {
        if (node->parent != nullptr) {
            auto verts = generateBranchVertices(node);
            vertexData.insert(vertexData.end(), verts.begin(), verts.end());
        }
    }

    std::vector<unsigned short> indicesData;
    indicesData.reserve(nodes.size());
    for (int i = 0; i < nodes.size(); i++) {
        int offset = i * settings.branchSides * 2;
        for (int j = 0; j < settings.branchSides; ++j) {
            indicesData.push_back(offset + (j * 2));
            indicesData.push_back(offset + (j * 2) + 3);
            indicesData.push_back(offset + (j * 2) + 2);

            indicesData.push_back(offset + (j * 2));
            indicesData.push_back(offset + (j * 2) + 1);
            indicesData.push_back(offset + (j * 2) + 3);
        }
    }
#else
    for (auto &node : nodes) {
        if (node->parent != nullptr) {
            vertexData.emplace_back(node->position);
            vertexData.emplace_back(node->parent->position);
        }
    }

    std::vector<unsigned short> indicesData;
    indicesData.reserve(nodes.size());
    for (auto i = 0; i < nodes.size(); i++) {
        indicesData.push_back(i);
    }
#endif

    indices = indicesData.size();

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
#ifdef CYLINDERS
    glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_SHORT, nullptr);
#else
    glDrawElements(GL_LINES, indices, GL_UNSIGNED_SHORT, nullptr);
#endif
}
