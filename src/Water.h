
#ifndef PROCGEN_WATER_H
#define PROCGEN_WATER_H


#include "Terrain.h"

class Water : public Terrain {
public:
    Water(unsigned short size, float maxRand, float h, Shader *shader, Material &material);

    void render() override;
};


#endif //PROCGEN_WATER_H
