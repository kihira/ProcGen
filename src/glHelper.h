
#ifndef PROCGEN_GLHELPER_H
#define PROCGEN_GLHELPER_H

#include <glad/glad.h>
#include <iostream>

#define GLERRCHECK() glErrorCheck(__FILE__, __LINE__)

namespace {
    void glErrorCheck(const char *file, int line) {
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cerr << "(" << file << ":" << line << ") OpenGL error 0x" << std::hex << err << std::endl;
        }
    }
}

#endif //PROCGEN_GLHELPER_H
