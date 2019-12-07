#version 330 core

in vec3 position;
in vec3 normal;
in vec2 uv;

uniform mat4 lightSpaceMatrix = mat4(1.0);
uniform mat4 transform = mat4(1.0);

void main()
{
    gl_Position = lightSpaceMatrix * transform * vec4(position, 1.0);
}  
