#version 330 core

struct Material {
    vec3 diffuse;
    vec3 specular;
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

uniform Material material;
uniform Light light;
uniform sampler2D textures[1];
uniform float minY;
uniform float maxY;

out vec4 colour;

void main() {
    vec3 lightDir = normalize(light.position);
    vec4 ambient = texture(textures[0], uv) * vec4(light.ambient, 1.f);
    vec4 diffuse;

    diffuse = texture(textures[0], uv);
    diffuse *= max(dot(normal, lightDir), 0.f);
    diffuse *= vec4(light.diffuse, 1.f);

    colour = ambient + diffuse;
}