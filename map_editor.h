#ifndef MAP_EDITOR_H
#define MAP_EDITOR_H

#include "render.h"

struct MapObject {
    Model model;
    std::string filepath;
    u64 flags;
};

struct MapEditor {
    std::vector<MapObject> entities;
    Camera cam;
    ModelBatch batch;
    mat4 projection;
};

static inline
vec3 point_to_normalized_device_space(vec2 point, Rect viewport) {
    f32 x = (2.0f * point.x) / viewport.width - 1.0f;
    f32 y = 1.0f - (2.0f * point.y) / viewport.height;
    f32 z = 1.0f;
    return {x, y, z};
}

static inline
vec3 create_raycast(mat4 projection, Camera cam, vec2 mouse, Rect viewport) {
    vec3 ray_nds = point_to_normalized_device_space(mouse, viewport);
    vec4 ray_clip = {ray_nds.x, ray_nds.y, -1.0f, 1.0f};
    vec4 ray_eye = inverse(projection) * ray_clip;
    ray_eye = {ray_eye.x, ray_eye.y, -1.0, 1.0};
    vec3 ray_world = (inverse(create_view_matrix(cam)) * ray_eye).xyz;
    normalize(&ray_world);
    return ray_world;
}

#endif
