#version 330 core

in vec3 uvw;

uniform samplerCube tex;

out vec4 colour;

void main() {
    colour = texture(tex, uvw);
}