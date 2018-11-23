
#ifndef PROCGEN_LIGHT_H
#define PROCGEN_LIGHT_H

#include <vec3.hpp>

struct Light {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

#endif //PROCGEN_LIGHT_H
