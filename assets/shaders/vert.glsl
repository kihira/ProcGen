#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
// layout(location = 2) in vec2 aUv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMat;

out float yPos;
out vec3 normal;
// out vec2 uv;

void main() {
    yPos = aPos.y;
    normal = normalize(normalMat * aNormal);
    // uv = aUv;
    gl_Position = projection * view * model * vec4(aPos, 1.f);
}