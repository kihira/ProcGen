
#include "Water.h"
#include <GLFW/glfw3.h>

Water::Water(unsigned short size, float maxRand, float h, Shader *shader, Material &material) : Terrain(size, maxRand,
                                                                                                        h, shader,
                                                                                                        material) {}

void Water::render() {
    shader->use();
    shader->setUniform("time", (float) glfwGetTime());
    Terrain::render();
}
