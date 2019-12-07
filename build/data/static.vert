#version 330 core

in vec3 position;
in vec3 normal;
in vec2 uv;

uniform mat4 projection = mat4(1.0);
uniform mat4 transform = mat4(1.0);
uniform mat4 view = mat4(1.0);
uniform vec3 light_pos = vec3(2, 4, 2);

out vec2 pass_uv;
out vec3 pass_normal;
out vec3 pass_pos;
out float light_dot;

void main() {
    pass_uv = uv;
    pass_normal = normal;
    pass_pos = vec3(transform * vec4(position, 1.0));

    vec4 world_position  = transform * vec4(position, 1.0);
    vec4 position_by_camera = view * world_position;

    vec3 to_light = light_pos - world_position.xyz;
    vec3 new_normal = normalize(normal);
    vec3 normal_light = normalize(to_light);
    light_dot = dot(normal_light, new_normal);

    gl_Position = projection * position_by_camera;
}
