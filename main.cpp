//
//
//  Author: Corbin Stark
//
//

#include "ENGINE/bahamut.h"
#include "ENGINE/window.h"
#include "ENGINE/maths.h"
#include "ENGINE/shader.h"
#include "ENGINE/texture.h"
#include "ENGINE/render2D.h"
#include <stdlib.h>
#include <time.h>
#include "render.h"
#include "map_editor.h"

#define RENDER_WIDTH 800.0f
#define RENDER_HEIGHT 600.0f

//
//  PROTOTYPES
//

void initialize();
void setup_environment();
void camera_controls(Camera* cam, vec2* lastPos);
Model generate_terrain(f32 width, f32 height);
Shader load_water_shader();

//
//  MAIN
//

const int WATER_WIDTH = 1600;
const int WATER_HEIGHT = 1400;

int main() {
    srand(time(NULL));
    initialize();

    //LOAD SHADERS
    Shader basic = load_shader_3D("data/shaders/static.vert", "data/shaders/static.frag");
    Shader water = load_water_shader();

    //CREATE QUAD BATCH FOR EFFICIENT GUI RENDERING
    QuadBatch* batch = &create_quad_batch();
    batch->shader = load_quad_shader();
    start_shader(batch->shader);
    upload_mat4(batch->shader, "projection", orthographic_projection(0, 0, get_window_width(), get_window_height(), -1, 1));
    stop_shader();

    //LOAD SCENE
    Model groundModel = generate_terrain(350, 350);
    std::vector<Model> scene;
    scene.push_back(load_model("data/models/ship_light.obj"));
    scene.push_back(load_model("data/models/ship_light.obj"));
    scene.push_back(load_model("data/models/ship_light.obj"));
    scene.push_back(load_model("data/models/palm_long.obj"));
    for(int i = 0; i < scene.size(); ++i) {
        scene[i].scale.x /= 4;
        scene[i].scale.y /= 4;
        scene[i].scale.z /= 4;
        scene[i].pos.x = (rand()%(100+100))-100;
        scene[i].pos.z = (rand()%(100+100))-100;
        scene[i].rotate.y = (rand()%360);
    }

    vec2 lastMousePos = {0};
    Camera cam = {0};
    cam.y = 5;

    Framebuffer inverse = create_color_buffer(WATER_WIDTH, WATER_HEIGHT, GL_LINEAR);
    Texture dudvMap = load_texture("data/textures/dudv.png", GL_LINEAR);
    float moveFactor = 0;

    while(window_open()) {
        camera_controls(&cam, &lastMousePos);

        for(int i = 0; i < scene.size(); ++i) {
            scene[i].rotate.y += 0.1;
            f32 theta = deg_to_rad(scene[i].rotate.y);
            scene[i].pos.z -= 0.05f * cos(theta);
            scene[i].pos.x -= 0.05f * sin(theta);
        }

        begin_drawing();
        setup_environment();

        mat4 projection = perspective_projection(90, get_window_width() / get_window_height(), 0.1f, 999.9f);

        //PREPARE BASIC SHADER
        start_shader(basic);
        upload_mat4(basic, "projection", projection);
        moveFactor += 0.0005f;

        //REFLECT CAMERA ACROSS WATER (Y-AXIS)
        float distance = 2 * cam.y;
        cam.y -= distance;
        cam.pitch = -cam.pitch;
        upload_mat4(basic, "view", create_view_matrix(cam));

        //RENDER INVERTED SCENE ONTO FRAMEBUFFER
        set_viewport(0, 0, WATER_WIDTH, WATER_HEIGHT);
        bind_framebuffer(inverse);
        clear_bound_framebuffer();
        for(Model m : scene)
            draw_model(basic, &m);
        unbind_framebuffer();

        //UN-REFLECT CAMERA
        cam.y += distance;
        cam.pitch = -cam.pitch;
        mat4 view = create_view_matrix(cam);
        upload_mat4(basic, "view", view);

        //DRAW SCENE TO SCREEN
        set_viewport(0, 0, get_window_width(), get_window_height());
        for(Model m : scene)
            draw_model(basic, &m);

        //DRAW WATER
        start_shader(water);
        bind_texture(inverse.texture, 0); //bind inverse framebuffer texture to texture slot 0
        bind_texture(dudvMap, 1); //bind dudv texture to texture slot 1
        upload_float(water, "moveFactor", sin(moveFactor));
        upload_mat4(water, "view", view);
        draw_mesh(water, groundModel.meshes[0]);

        //DRAW GUI
        bind_quad_batch(batch);
            //draw_texture(batch, dudvMap, 0, 0);
        unbind_quad_batch(batch);

        end_drawing();
    }
}

//
//  FUNCTIONS
//

void initialize() {
    //INITIALIZE WINDOW
    printf("\n/////////////////////////////////\n       BAHAMUT ENGINE\n/////////////////////////////////\n\n");
    init_window(RENDER_WIDTH, RENDER_HEIGHT, "OpenGL - Bahamut Engine", false, true, true);
    set_fps_cap(60);
    set_vsync(true);
    set_clear_color(SKYBLUE);
    set_mouse_state(MOUSE_LOCKED);

    setup_environment();
}

void setup_environment() {
    //SET GLOBAL OPENGL STATES
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);
}

void camera_controls(Camera* cam, vec2* lastPos) {
    static bool look = true;
    const float speed = 0.7;

    //CAMERA CONTROLS
    if(look) {
        vec2 mousePos = get_mouse_pos();
        cam->yaw += (mousePos.x - lastPos->x) * 0.15f;
        cam->pitch += (mousePos.y - lastPos->y) * 0.15f;
        *lastPos = mousePos;
        if (is_key_down(KEY_A)) {
            move_cam_left(cam, speed);
        }
        if (is_key_down(KEY_D)) {
            move_cam_right(cam, speed);
        }
        if (is_key_down(KEY_S)) {
            move_cam_backwards(cam, speed);
        }
        if (is_key_down(KEY_W)) {
            move_cam_forward(cam, speed);
        }
    }

    if(is_key_released(KEY_ESCAPE))
        exit(0);

    if(is_key_released(KEY_F1)) {
        if(look)
            set_mouse_state(MOUSE_NORMAL);
        else
            set_mouse_state(MOUSE_LOCKED);
        *lastPos = get_mouse_pos();
        look = !look;
    }
}

Model generate_terrain(f32 width, f32 height) {
    Model model;
    model.pos = {0};
    model.rotate = {180, 0, 0};
    model.scale = {1, 1, 1};

    std::vector<ColorVertex> vertices;
    std::vector<GLushort>    indices;

    ColorVertex triOne[3];
    ColorVertex triTwo[3];

    vec3 topLeft = {-width, 0, -height};
    vec3 topRight = {width, 0, -height};
    vec3 bottomLeft = {-width, 0, height};
    vec3 bottomRight = {width, 0, height};

    vec3 normal = {0, 1, 0};
    vec4 color = {88.0f/255.0f, 213.0f/255.0f, 211.0f/255.0f};

    triOne[0] = { topLeft, normal, color };
    triOne[1] = { topRight, normal, color };
    triOne[2] = { bottomLeft, normal, color };

    triTwo[0] = { bottomLeft, normal, color };
    triTwo[1] = { topRight, normal, color };
    triTwo[2] = { bottomRight, normal, color };

    vertices.push_back(triOne[0]);
    vertices.push_back(triOne[1]);
    vertices.push_back(triOne[2]);
    vertices.push_back(triTwo[0]);
    vertices.push_back(triTwo[1]);
    vertices.push_back(triTwo[2]);

    indices.push_back(0);
    indices.push_back(0);
    indices.push_back(0);
    indices.push_back(0);
    indices.push_back(0);
    indices.push_back(0);

    model.meshes.push_back(create_color_mesh(vertices, indices));

    return model;
}

Shader load_water_shader() {
    Shader shader = { 0 };
    shader.vertexshaderID = load_shader_file("data/shaders/water.vert", GL_VERTEX_SHADER);
    shader.fragshaderID = load_shader_file("data/shaders/water.frag", GL_FRAGMENT_SHADER);
    shader.ID = glCreateProgram();

    glAttachShader(shader.ID, shader.vertexshaderID);
    glAttachShader(shader.ID, shader.fragshaderID);
    glBindFragDataLocation(shader.ID, 0, "out_color");
    glBindAttribLocation(shader.ID, 0, "position");
    glBindAttribLocation(shader.ID, 1, "normal");
    glBindAttribLocation(shader.ID, 2, "color");
    glLinkProgram(shader.ID);
    glValidateProgram(shader.ID);

    start_shader(shader);
    upload_int(shader, "reflection", 0);
    upload_int(shader, "dudv", 1);
    upload_mat4(shader, "transform", create_transformation_matrix({0, 0, 0}, {180, 0, 0}, {1, 1, 1}));
    upload_mat4(shader, "projection", perspective_projection(90, WATER_WIDTH / WATER_HEIGHT, 0.1f, 999.9f));

    glUseProgram(0);
    return shader;
}
