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

in float yPos;
in vec3 normal;
in vec2 uv;

uniform Material material;
uniform Light light;
uniform sampler2D textures[2];
uniform float minY;
uniform float maxY;

out vec4 colour;

void main() {
    vec3 lightDir = normalize(light.position);
    vec4 diffuse;

    const float endSand = 0.35f;
    const float startGrassRocks = 0.4f;

    float yScale = yPos - minY;
    yScale /= maxY - minY;

    // Sand only
    if (yScale < endSand) {
        diffuse = texture(textures[0], uv);
    }
    // Mix sand and grass
    else if (yScale < startGrassRocks) {
        yScale -= endSand;
        yScale /= startGrassRocks - endSand;

        diffuse = mix(texture(textures[0], uv), texture(textures[1], uv), yScale);
    }
    else {
        diffuse = texture(textures[1], uv);
    }
    // Can reuse the diffuse value to calculate ambient
    vec4 ambient = diffuse * vec4(light.ambient, 1.f);
    diffuse *= max(dot(normal, lightDir), 0.f);
    diffuse *= vec4(light.diffuse, 1.f);

    colour = ambient + diffuse;
}