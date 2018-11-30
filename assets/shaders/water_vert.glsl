#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMat;

out vec3 normal;
out vec2 uv;

void main() {
    normal = normalize(normalMat * aNormal);
    uv = aUv;
    vec4 position = projection * view * model * vec4(aPos, 1.f);
    position.y += sin(position.x);
    gl_Position = position;
}