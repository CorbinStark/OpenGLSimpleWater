#version 330 core

out vec3 color;

in vec2 pass_uv;
in vec3 pass_normal;
in vec3 pass_pos;
in float light_dot;

uniform vec4 diffuseColor = vec4(1, 1, 1, 1);
uniform vec3 lightColor = vec3(1, 1, 1);

void main() {
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    float brightness = light_dot;
    brightness = max(brightness, 0);
    vec3 diffuse = brightness * lightColor;

    color = (ambient + diffuse) * diffuseColor.xyz;
}
