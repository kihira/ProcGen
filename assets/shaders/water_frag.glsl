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

in vec3 normal;
in vec2 uv;

uniform vec3 globalAmbient;
uniform Material material;
uniform Light light;
uniform sampler2D textures[1];
uniform float minY;
uniform float maxY;

out vec4 colour;

void main() {
    vec3 lightDir = normalize(light.position);
    vec4 diffuse = vec4(material.diffuse * light.diffuse, 1.f);

    diffuse *= texture(textures[0], uv);

    colour = max(dot(normal, lightDir), 0.f) * diffuse;
}