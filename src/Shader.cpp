
#include <iostream>
#include <fstream>
#include <ext.hpp>
#include "Shader.h"
#include "Mesh.h"
#include "Light.h"

Shader::Shader(const char *vertexFile, const char *fragFile) {
    // Load shaders from source
    std::ifstream file;
    std::string line;
    std::string vertexSrc, fragmentSrc;

    // Load vertex source
    file.open(vertexFile, std::ios::in);
    if (!file.is_open()) {
        std::cerr << "Failed to open vertex file" << std::endl;
        return;
    }
    while (getline(file, line)) {
        vertexSrc += line + "\n";
    }
    file.close();

    // Load fragment source
    file.open(fragFile, std::ios::in);
    if (!file.is_open()) {
        std::cerr << "Failed to open fragment file" << std::endl;
        return;
    }
    while (getline(file, line)) {
        fragmentSrc += line + "\n";
    }
    file.close();

    // Compile shaders
    GLuint vertexShader, fragmentShader;
    vertexShader = createShader(GL_VERTEX_SHADER, vertexSrc.c_str());
    fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentSrc.c_str());

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

void Shader::setUniform(const char *name, glm::mat3 value) {
    glUniformMatrix3fv(glGetUniformLocation(program, name), 1, GL_FALSE, glm::value_ptr(value));
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

void Shader::setUniform(const char *name, int value) {
    glUniform1i(glGetUniformLocation(program, name), value);
}
