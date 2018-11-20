
#ifndef PROCGEN_SHADER_H
#define PROCGEN_SHADER_H


#include <glad/glad.h>
#include <glm.hpp>

class Shader {
private:
    GLuint program;

    GLuint createShader(GLenum type, const char *src);

    bool checkShaderCompile(GLuint shader);
public:
    Shader(const char *vertexSrc, const char *fragSrc);

    void use();

    void setUniform(const char *name, glm::mat4 value);
};


#endif //PROCGEN_SHADER_H
