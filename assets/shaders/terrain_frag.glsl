#version 330 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 emissive;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in float yPos;
in vec3 normal;
// in vec2 uv;

uniform vec3 globalAmbient;
uniform Material material;
uniform Light light;

out vec4 colour;

void main() {
    vec3 lightDir = normalize(vec3(light.position));
    vec3 diffuse = material.diffuse * light.diffuse;

    if (yPos < -10.f) {
        diffuse = vec3(0.93f, 0.788f, 0.686f);
    }

    colour = vec4(max(dot(normal, lightDir), 0.f) * diffuse, 1);
}