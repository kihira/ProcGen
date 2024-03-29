
#ifndef PROCGEN_SHADER_H
#define PROCGEN_SHADER_H


#include <glm.hpp>
#include <glad/glad.h>

class Material; // Forward deceleration
class Light;

class Shader {
private:
    GLuint program;

    GLuint createShader(GLenum type, const char *src);

    bool checkShaderCompile(GLuint shader);
public:
    Shader(const char *vertexFile, const char *fragFile);

    void use();

    template <typename T>
    void setUniform(const char *name, T value);

    void setMaterial(Material &material);

    void setGlobalAmbient(glm::vec3 &colour);

    void setLight(const Light &light);
};


#endif //PROCGEN_SHADER_H
