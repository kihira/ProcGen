
#include "Tree.h"
#include <random>
#include <geometric.hpp>
#include <ext/matrix_transform.hpp>
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/rotate_vector.hpp>
#include <gtx/quaternion.hpp>

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
            auto newNode = new Node(node, node->position + (direction * settings.nodeSize), direction);
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

// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/#how-do-i-find-the-rotation-between-2-vectors-
glm::quat rotationBetweenVectors(glm::vec3 start, glm::vec3 dest){
    start = normalize(start);
    dest = normalize(dest);

    float cosTheta = dot(start, dest);
    glm::vec3 rotationAxis;

    if (cosTheta < -1 + 0.001f){
        // special case when vectors in opposite directions:
        // there is no "ideal" rotation axis
        // So guess one; any will do as long as it's perpendicular to start
        rotationAxis = cross(glm::vec3(0.0f, 0.0f, 1.0f), start);
        if (glm::length(rotationAxis) < 0.01 ) // bad luck, they were parallel, try again!
            rotationAxis = cross(glm::vec3(1.0f, 0.0f, 0.0f), start);

        rotationAxis = normalize(rotationAxis);
        return glm::angleAxis(glm::radians(180.0f), rotationAxis);
    }

    rotationAxis = cross(start, dest);

    float s = sqrt( (1+cosTheta)*2 );
    float invs = 1 / s;

    return glm::quat(
            s * 0.5f,
            rotationAxis.x * invs,
            rotationAxis.y * invs,
            rotationAxis.z * invs
    );

}

std::vector<glm::vec3> Tree::generateBranchVertices(Node *node) {
    std::vector<glm::vec3> vertices;
    vertices.reserve(settings.branchSides * 2);
    auto segmentSize = glm::radians(360.f / static_cast<float>(settings.branchSides));
    glm::mat4 bottomTransform = glm::translate(node->parent->position);
    //glm::mat4 bottomRotation(1.f);
    glm::mat4 bottomRotation = glm::toMat4(rotationBetweenVectors(node->parent->direction, node->direction));
    glm::mat4 topTransform = glm::translate(node->position);
    //glm::mat4 topRotation(1.f);
    glm::mat4 topRotation = glm::toMat4(rotationBetweenVectors(node->parent->direction, node->direction));

    for (int i = 0; i < settings.branchSides; ++i) {
        auto bottomVertex = glm::vec3(0.f, 0.f, settings.nodeSize * .5f);
        bottomVertex = glm::rotateY(bottomVertex, segmentSize * i);
        bottomVertex = bottomTransform * bottomRotation * glm::vec4(bottomVertex, 1.f);
        vertices.emplace_back(bottomVertex);

        auto topVertex = glm::vec3(0.f, 0.f, settings.nodeSize * .5f);
        topVertex = glm::rotateY(topVertex, segmentSize * i);
        topVertex = topTransform * topRotation * glm::vec4(topVertex, 1.f);
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
    indicesData.reserve(nodes.size() * settings.branchSides * 2);
    for (int i = 0; i < nodes.size() - 1; i++) {
        int offset = indicesData.size();
        for (int j = 0; j < settings.branchSides; j++) {
            indicesData.push_back(offset + ((j * 2) % (settings.branchSides * 2)));
            indicesData.push_back(offset + (((j * 2) + 3) % (settings.branchSides * 2)));
            indicesData.push_back(offset + (((j * 2) + 2) % (settings.branchSides * 2)));

            indicesData.push_back(offset + ((j * 2) % (settings.branchSides * 2)));
            indicesData.push_back(offset + (((j * 2) + 1) % (settings.branchSides * 2)));
            indicesData.push_back(offset + (((j * 2) + 3) % (settings.branchSides * 2)));
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
