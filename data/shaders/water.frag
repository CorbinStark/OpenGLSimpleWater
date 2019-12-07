#version 330 core

in vec3 pass_color;
in vec4 clip_space;
in vec2 pass_uv;
out vec4 out_color;

uniform sampler2D reflection;
uniform sampler2D dudv;
uniform float moveFactor;

//
//  MAIN
//

const float waveStrength = 0.005;

void main(void) {
    vec2 ndc = (clip_space.xy / clip_space.w) / 2.0 + 0.5;
    vec2 reflectionUV = vec2(ndc.x, 1.0-ndc.y);
    vec2 distortion = (texture(dudv, vec2(pass_uv.x + moveFactor, pass_uv.y)).rg * 2.0 - 1.0) * waveStrength;
    reflectionUV += distortion;
    vec4 reflectionColor = texture(reflection, reflectionUV);

    out_color = mix(reflectionColor, vec4(pass_color, 1.0), 0.9);
}
