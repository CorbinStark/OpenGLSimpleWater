#version 330 core

in vec3 position;
in vec3 normal;
in vec4 color;

out vec3 pass_color;

uniform mat4 projection = mat4(1.0);
uniform mat4 transform = mat4(1.0);
uniform mat4 view = mat4(1.0);

uniform vec3 lightPos = vec3(-2, 4, -1);
uniform vec3 lightColor = vec3(1.0, 1.0, 1.0);
uniform vec2 lightBias = vec2(1.0, 1.0);

vec3 calculate_light() {
    vec3 lightDir = normalize(lightPos - position);

    vec3 light_normal = normal.xyz * 2.0 - 1.0;
    float brightness = max(dot(-lightDir, light_normal), 0.0);
    return (lightColor * lightBias.x) + (brightness * lightColor * lightBias.y);
}

void main(void) {
    gl_Position = projection * view * transform * vec4(position, 1.0);

    vec3 lighting = calculate_light();
    pass_color = color.rgb * lighting;
}
