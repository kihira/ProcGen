
#include <iostream>
#include <ext.hpp>
#include "Shader.h"
#include "Mesh.h"
#include "Light.h"

Shader::Shader(const char *vertexSrc, const char *fragSrc) {
    GLuint vertexShader, fragmentShader;

    // Compile shaders
    vertexShader = createShader(GL_VERTEX_SHADER, vertexSrc);
    fragmentShader = createShader(GL_FRAGMENT_SHADER, fragSrc);

    // Create program
    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // Check program link
    GLint err;
    glGetProgramiv(program, GL_LINK_STATUS, &err);
    if (err != GL_TRUE) {
        GLchar errData[1024];
        glGetProgramInfoLog(program, 1024, nullptr, errData);
        std::cerr << errData << std::endl;
    }
}

bool Shader::checkShaderCompile(GLuint shader) {
    GLint err;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &err);
    if (err != GL_TRUE) {
        GLchar errData[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, errData);
        std::cerr << errData << std::endl;
        return false;
    }
    return true;
}

GLuint Shader::createShader(GLenum type, const char *src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    checkShaderCompile(shader);
    return shader;
}

void Shader::use() {
    glUseProgram(program);
}

void Shader::setUniform(const char *name, glm::mat4 value) {
    glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setMaterial(Material &material) {
    glUniform3fv(glGetUniformLocation(program, "material.ambient"), 1, glm::value_ptr(material.ambient));
    glUniform3fv(glGetUniformLocation(program, "material.diffuse"), 1, glm::value_ptr(material.ambient));
    glUniform3fv(glGetUniformLocation(program, "material.specular"), 1, glm::value_ptr(material.ambient));
    glUniform3fv(glGetUniformLocation(program, "material.emmisive"), 1, glm::value_ptr(material.ambient));
    glUniform1f(glGetUniformLocation(program, "material.shininess"), material.shininess);
}

void Shader::setGlobalAmbient(glm::vec3 &colour) {
    glUniform3fv(glGetUniformLocation(program, "globalAmbient"), 1, glm::value_ptr(colour));
}

void Shader::setLight(const Light &light) {
    glUniform3fv(glGetUniformLocation(program, "light.position"), 1, glm::value_ptr(light.position));
    glUniform3fv(glGetUniformLocation(program, "light.ambient"), 1, glm::value_ptr(light.ambient));
    glUniform3fv(glGetUniformLocation(program, "light.diffuse"), 1, glm::value_ptr(light.diffuse));
    glUniform3fv(glGetUniformLocation(program, "light.specular"), 1, glm::value_ptr(light.specular));
}
