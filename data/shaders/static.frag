#version 330 core

out vec4 color;

in vec2 pass_uv;
in vec3 pass_normal;
in vec3 pass_pos;
in vec4 pass_lightspace;

uniform sampler2D shadowMap;

uniform vec4 diffuseColor = vec4(1, 1, 1, 1);
uniform vec3 lightColor = vec3(1, 1, 1);
uniform vec3 lightPos = vec3(0, 40, 0);

//
//FUNCTIONS
//

float shadow_calculation(vec4 lightspace, vec3 lightDir) {
    vec3 projCoords = lightspace.xyz / lightspace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = max(0.05 * (1.0f - dot(pass_normal, lightDir)), 0.005f);

    float shadow = 0.0f;
    vec2 texelSize = 1.0f / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0f;
    //float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;

    return shadow;
}

//
//   MAIN
//

void main() {
    vec3 normal = normalize(pass_normal);
    vec3 ambient = 0.65 * diffuseColor.xyz;
    vec3 lightDir = normalize(lightPos - pass_pos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;

    float shadow = shadow_calculation(pass_lightspace, lightDir);
    //vec3 lighting = (ambient + (1.0f - shadow) * (diffuse)) * diffuseColor.rgb;
    vec3 lighting = (ambient + diffuse) * diffuseColor.xyz;
    color = vec4(lighting, 1.0);
}
