#ifndef RENDER_H
#define RENDER_H

#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <unordered_map>
#include "ENGINE/maths.h"
#include "ENGINE/texture.h"
#include "ENGINE/shader.h"

#define INVALID_MATERIAL 0xFFFFFFFF

struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 uv;
};

struct ColorVertex {
    vec3 position;
    vec3 normal;
    vec4 color;
};

struct Material {
    Texture diffuse;
    Texture normals;
    Texture specular;
    vec4 diffuseColor;
    vec4 ambientColor;
    vec4 specularColor;
    f32 gloss;
};

struct Mesh {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    u32 indexcount;
    u32 material;
};

struct Model {
    std::vector<Mesh> meshes;
    std::vector<Material> materials;

    vec3 pos;
    vec3 rotate;
    vec3 scale;
};  

struct ModelBatch {
    std::unordered_map<GLuint, std::vector<Model>> drawpool;
    Shader shader;
};

static inline
void dispose_mesh(Mesh* mesh) {
    glDeleteBuffers(1, &mesh->vbo);
    glDeleteBuffers(1, &mesh->ebo);
    glDeleteVertexArrays(1, &mesh->vao);
    mesh->indexcount = mesh->material = 0;
}

static inline
void dispose_model(Model* model) {
    for (u32 i = 0; i < model->meshes.size(); ++i)
        dispose_mesh(&model->meshes[i]);
    model->meshes.clear();
    model->materials.clear();
}

static inline
Mesh create_mesh(std::vector<Vertex> vertices, std::vector<GLushort> indices) {
    Mesh mesh = {0};

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)0);                     //position
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(3 * sizeof(GLfloat))); //normals
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(6 * sizeof(GLfloat))); //tex coords

    glGenBuffers(1, &mesh.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indices.size(), &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    mesh.indexcount = indices.size();

    return mesh;
}

static inline
Mesh create_color_mesh(std::vector<ColorVertex> vertices, std::vector<GLushort> indices) {
    Mesh mesh = {0};

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ColorVertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ColorVertex), (const GLvoid*)0);                     //position
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ColorVertex), (const GLvoid*)(3 * sizeof(GLfloat))); //normals
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(ColorVertex), (const GLvoid*)(6 * sizeof(GLfloat))); //color

    glGenBuffers(1, &mesh.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indices.size(), &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    mesh.indexcount = indices.size();

    return mesh;
}

static inline
void load_mesh(Model* model, u32 i, const aiMesh* paiMesh) {
    model->meshes[i].material = paiMesh->mMaterialIndex;

    std::vector<Vertex> vertices;
    std::vector<GLushort> indices;

    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

    for(u32 i = 0; i < paiMesh->mNumVertices; ++i) {
        const aiVector3D* pos = &(paiMesh->mVertices[i]);
        const aiVector3D* normal = &(paiMesh->mNormals[i]);
        const aiVector3D* uv = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

        Vertex v = {
            {pos->x, pos->y, pos->z},
            {normal->x, normal->y, normal->z},
            {uv->x, uv->y}
        };

        vertices.push_back(v);
    }

    for(u32 i = 0; i < paiMesh->mNumFaces; ++i) {
        const aiFace& face = paiMesh->mFaces[i];
        assert(face.mNumIndices == 3);
        indices.push_back(face.mIndices[0]);
        indices.push_back(face.mIndices[1]);
        indices.push_back(face.mIndices[2]);
    }

    glGenVertexArrays(1, &model->meshes[i].vao);
    glBindVertexArray(model->meshes[i].vao);

    glGenBuffers(1, &model->meshes[i].vbo);
    glBindBuffer(GL_ARRAY_BUFFER, model->meshes[i].vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)0);                     //vertices
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(3 * sizeof(GLfloat))); //normals
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(6 * sizeof(GLfloat))); //tex coords

    glGenBuffers(1, &model->meshes[i].ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->meshes[i].ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indices.size(), &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    model->meshes[i].indexcount = indices.size();
}

static inline
void load_materials(Model* model, const aiScene* pScene, const char* filename) {
    for(u32 i = 0; i < pScene->mNumMaterials; ++i) {
        const aiMaterial* mat = pScene->mMaterials[i];
        model->materials[i] = {0};

        //diffuse
        if(mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString path;

            if(mat->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                std::string fullpath = "data/art/";
                fullpath.append(path.data);
                model->materials[i].diffuse = load_texture(fullpath.c_str(), GL_LINEAR);
            }
        }
        aiColor4D diffuseColor;
        if(aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor) == AI_SUCCESS)
            model->materials[i].diffuseColor = {(f32)diffuseColor.r, (f32)diffuseColor.g, (f32)diffuseColor.b, (f32)diffuseColor.a};

        //specular
        if(mat->GetTextureCount(aiTextureType_SPECULAR) > 0) {
            aiString path;

            if(mat->GetTexture(aiTextureType_SPECULAR, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                std::string fullpath = "data/art/";
                fullpath.append(path.data);
                model->materials[i].specular = load_texture(fullpath.c_str(), GL_LINEAR);
            }
        }
        aiColor4D specularColor;
        if(aiGetMaterialColor(mat, AI_MATKEY_COLOR_SPECULAR, &specularColor) == AI_SUCCESS)
            model->materials[i].specularColor = {(f32)specularColor.r, (f32)specularColor.g, (f32)specularColor.b, (f32)specularColor.a};

        //ambient
        aiColor4D ambientColor;
        if(aiGetMaterialColor(mat, AI_MATKEY_COLOR_AMBIENT, &ambientColor) == AI_SUCCESS)
            model->materials[i].ambientColor = {(f32)ambientColor.r, (f32)ambientColor.g, (f32)ambientColor.b, (f32)ambientColor.a};
    }
}

static inline
Model load_model(const char* filename) {
    Model model;
    model.pos = {0};
    model.rotate = {0};
    model.scale = {1, 1, 1};

    Assimp::Importer importer;
    const aiScene* pScene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_GenNormals);

    if(pScene) {
        model.meshes.resize(pScene->mNumMeshes);
        model.materials.resize(pScene->mNumMaterials);

        for(u32 i = 0; i < model.meshes.size(); ++i) {
            aiMesh* paiMesh = pScene->mMeshes[i];
            load_mesh(&model, i, paiMesh);
        }
    }
    else {
        printf("Error loading model\n");
    }

    load_materials(&model, pScene, filename);
    return model;
}

static inline
void draw_mesh(Shader shader, Mesh mesh) {
    //bind VERTEX ARRAY OBJECT
    //and all attributes of it
    glBindVertexArray(mesh.vao);
    glEnableVertexAttribArray(0); //0 = Position
    glEnableVertexAttribArray(1); //1 = Texture Coordinates
    glEnableVertexAttribArray(2); //2 = Color

    //draw bound VAO using triangles, up to mesh.indexcount indices
    //glDrawElements(GL_TRIANGLES, mesh.indexcount, GL_UNSIGNED_SHORT, 0);
    glDrawArrays(GL_TRIANGLES, 0, mesh.indexcount);

    //unbind attributes and VAO
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    glBindVertexArray(0);
}

static inline
void draw_mesh(Shader basic, Mesh mesh, Material material) {
    //bind VERTEX ARRAY OBJECT
    //and all attributes of it
    glBindVertexArray(mesh.vao);
    glEnableVertexAttribArray(0); //0 = Position
    glEnableVertexAttribArray(1); //1 = Texture Coordinates
    glEnableVertexAttribArray(2); //2 = Normals

    //the models in this example are all low-poly and minimalist color, each mesh has 1 color and no texture
    //upload the color (vec4) to the shader
    upload_vec4(basic, "diffuseColor", material.diffuseColor);
    //draw bound VAO using triangles, up to mesh.indexcount indices
    glDrawElements(GL_TRIANGLES, mesh.indexcount, GL_UNSIGNED_SHORT, 0);

    //unbind attributes and VAO
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    glBindVertexArray(0);
}

static inline
void draw_model(Shader shader, Model* model) {
        //UPLOAD MODEL MATRIX
        upload_mat4(shader, "transform", create_transformation_matrix(model->pos, model->rotate, model->scale));

        //ONE MATERIAL PER MESH -- DRAW ALL MESHES WITH THEIR MATERIALS (NO TEXTURES IN THESE LOW POLY MODELS, ONLY DIFFUSE COLOR)
        for(Mesh mesh : model->meshes) {
            draw_mesh(shader, mesh, model->materials[mesh.material]);
        }
}

/*
static inline
void begin3D(ModelBatch* batch) {
    start_shader(batch->shader);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    batch->drawpool.clear();
}


static inline
void draw_model(ModelBatch* batch, Model model, vec3 pos, vec3 rotate, vec3 scale) {
    model.pos = pos;
    model.rotate = rotate;
    model.scale = scale;
    batch->drawpool[model.texture.id].push_back(model);
}

static inline
void draw_model(ModelBatch* batch, Model model) {
    draw_model(batch, model, model->pos, model->rotate, model->scale);
}
static inline
void draw_model(ModelBatch* batch, Model model, vec3 pos) {
    draw_model(batch, model, pos, model->rotate, model->scale);
}
static inline
void draw_model(ModelBatch* batch, Model model, vec3 pos, vec3 rotate) {
    draw_model(batch, model, pos, rotate, model->scale);
}

static inline
void draw_model(ModelBatch* batch, Model model, f32 x, f32 y, f32 z) {
    draw_model(batch, model, {x, y, z});
}

static inline
void end3D(ModelBatch* batch) {
    for(current : batch->drawpool) {
        std::vector<Model>* modelList = &current.second;
        bind_texture(modelList->at(0).texture, 0);
        upload_int(batch->shader, "tex", 0);

        for(u16 i = 0; i < modelList->size(); ++i) {
            Model* model = &modelList->at(i);
            upload_mat4(batch->shader, "transform", create_transformation_matrix(model->pos, model->rotate, model->scale);

            for(u16 i = 0; i < model->meshes.size(); ++i) {
                Mesh* mesh = &model->meshes[i];

                glBindVertexArray(mesh->vao);
                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);
                glEnableVertexAttribArray(2);

                if(mesh->material < model->materials.size()) {
                    //bind_texture(model->materials[mesh->material].diffuse, 0);
                    upload_vec4(shader, "diffuseColor", model->materials[mesh->material].diffuseColor);
                }
                glDrawElements(GL_TRIANGLES, mesh->indexcount, GL_UNSIGNED_SHORT, 0);

                glDisableVertexAttribArray(2);
                glDisableVertexAttribArray(1);
                glDisableVertexAttribArray(0);
                glBindVertexArray(0);
            }
        }
        unbind_texture(0);
    }
    stop_shader();
}

*/

#endif
