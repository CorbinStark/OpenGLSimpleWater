#version 330 core

in vec3 position;
in vec3 normal;
in vec4 color;

out vec3 pass_color;
out vec4 clip_space;
out vec2 pass_uv;

uniform mat4 projection = mat4(1.0);
uniform mat4 transform = mat4(1.0);
uniform mat4 view = mat4(1.0);

//
//  MAIN
//

void main(void) {
    clip_space = projection * view * transform * vec4(position, 1.0);
    gl_Position = clip_space;
    pass_uv = vec2(position.x/2.0 + 0.5, position.z/2.0 + 0.5) / 350;
    pass_color = color.rgb;
}
