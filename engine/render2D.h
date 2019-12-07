///////////////////////////////////////////////////////////////////////////
// FILE:                       render2D.h                                //
///////////////////////////////////////////////////////////////////////////
//                      BAHAMUT GRAPHICS LIBRARY                         //
//                        Author: Corbin Stark                           //
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Corbin Stark                                       //
//                                                                       //
// Permission is hereby granted, free of charge, to any person obtaining //
// a copy of this software and associated documentation files (the       //
// "Software"), to deal in the Software without restriction, including   //
// without limitation the rights to use, copy, modify, merge, publish,   //
// distribute, sublicense, and/or sell copies of the Software, and to    //
// permit persons to whom the Software is furnished to do so, subject to //
// the following conditions:                                             //
//                                                                       //
// The above copyright notice and this permission notice shall be        //
// included in all copies or substantial portions of the Software.       //
//                                                                       //
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       //
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    //
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.//
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  //
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  //
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     //
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                //
///////////////////////////////////////////////////////////////////////////

#ifndef RENDER2D_H
#define RENDER2D_H

#include "defines.h"
#include "shader.h"
#include "texture.h"

struct VertexData {
	vec2 pos;
	vec4 color; //32 bit color (8 for R, 8 for G, 8 for B, 8 for A)
	vec2 uv;
	f32 texid;
};

#ifndef BATCH_MAX_SPRITES
#define BATCH_MAX_SPRITES	    20000
#endif

#define BATCH_VERTEX_SIZE	    sizeof(VertexData)
#define BATCH_SPRITE_SIZE	    BATCH_VERTEX_SIZE * 4
#define BATCH_BUFFER_SIZE	    BATCH_SPRITE_SIZE * BATCH_MAX_SPRITES
#define BATCH_INDICE_SIZE	    BATCH_MAX_SPRITES * 6
#define BATCH_MAX_TEXTURES		32

struct QuadBatch {
	u32 vao;
	u32 vbo;
	u32 ebo;
	u16 indexcount;
	u16 texcount;
	GLuint  textures[BATCH_MAX_TEXTURES];
	VertexData* buffer;
	Shader shader;
};

void unbind_quad_batch(QuadBatch* batch);

INTERNAL inline
QuadBatch create_quad_batch() {
	QuadBatch batch = { 0 };

	glGenVertexArrays(1, &batch.vao);
	glBindVertexArray(batch.vao);

	glGenBuffers(1, &batch.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, batch.vbo);
	glBufferData(GL_ARRAY_BUFFER, BATCH_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);

	//the last argument to glVertexAttribPointer is the offset from the start of the vertex to the
	//data you want to look at - so each new attrib adds up all the ones before it.
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, BATCH_VERTEX_SIZE, (const GLvoid*)0);                     //vertices
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, BATCH_VERTEX_SIZE, (const GLvoid*)(2 * sizeof(GLfloat))); //color
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, BATCH_VERTEX_SIZE, (const GLvoid*)(6 * sizeof(GLfloat))); //tex coords
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, BATCH_VERTEX_SIZE, (const GLvoid*)(8 * sizeof(GLfloat))); //texture id

	GLushort indices[BATCH_INDICE_SIZE];

	int offset = 0;
	for (u32 i = 0; i < BATCH_INDICE_SIZE; i += 6) {
		indices[i] = offset + 0;
		indices[i + 1] = offset + 1;
		indices[i + 2] = offset + 2;
		indices[i + 3] = offset + 2;
		indices[i + 4] = offset + 3;
		indices[i + 5] = offset + 0;

		offset += 4;
	}

	glGenBuffers(1, &batch.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, BATCH_INDICE_SIZE * sizeof(GLushort), indices, GL_STATIC_DRAW);

	//the vao must be unbound before the buffers
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return batch;
}

INTERNAL inline
void bind_quad_batch(QuadBatch* batch, bool blending = true, bool depthTest = false) {
	start_shader(batch->shader);

	if (blending)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
	if (depthTest)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	glBindBuffer(GL_ARRAY_BUFFER, batch->vbo);
	batch->buffer = (VertexData*)glMapBufferRange(GL_ARRAY_BUFFER, 0, BATCH_BUFFER_SIZE,
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
	);
}

INTERNAL inline
i32 submit_tex(QuadBatch* batch, Texture tex) {
	int texSlot = 0;
	bool found = false;
	for (u32 i = 0; i < batch->texcount; ++i) {
		if (batch->textures[i] == tex.ID) {
			texSlot = (i + 1);
			found = true;
			break;
		}
	}
	if (!found) {
		if (batch->texcount >= BATCH_MAX_TEXTURES) {
			unbind_quad_batch(batch);
			bind_quad_batch(batch);
		}
		batch->textures[batch->texcount++] = tex.ID;
		texSlot = batch->texcount;
	}
	return texSlot;
}

INTERNAL inline
void draw_texture(QuadBatch* batch, Texture tex, i32 xPos, i32 yPos, f32 r, f32 g, f32 b, f32 a) {
	if (tex.ID == 0)
		return;
	i32 texSlot = submit_tex(batch, tex);

	f32 x = (f32)xPos;
	f32 y = (f32)yPos;

	LOCAL f32 DEFAULT_UVS[8] = {0, 0, 0, 1, 1, 1, 1, 0};
	LOCAL f32 FLIP_HOR_UVS[8] = {1, 1, 1, 0, 0, 0, 0, 1};
	LOCAL f32 FLIP_VER_UVS[8] = {0, 1, 0, 0, 1, 0, 1, 1};
	LOCAL f32 FLIP_BOTH_UVS[8] = {1, 0, 1, 1, 0, 1, 0, 0};

	GLfloat* uvs;
	uvs = DEFAULT_UVS;
	if (tex.flip_flag & FLIP_HORIZONTAL && tex.flip_flag & FLIP_VERTICAL)
		uvs = FLIP_BOTH_UVS;
	if (tex.flip_flag & FLIP_HORIZONTAL)
		uvs = FLIP_HOR_UVS;
	if (tex.flip_flag & FLIP_VERTICAL)
		uvs = FLIP_VER_UVS;

	batch->buffer->pos = {x, y};
	batch->buffer->color = {r, g, b, a};
	batch->buffer->uv = {uvs[0], uvs[1]};
	batch->buffer->texid = texSlot;
	batch->buffer++;

	batch->buffer->pos = {x, y + tex.height};
	batch->buffer->color = {r, g, b, a};
	batch->buffer->uv = {uvs[2], uvs[3]};
	batch->buffer->texid = texSlot;
	batch->buffer++;

	batch->buffer->pos = {x + tex.width, y + tex.height};
	batch->buffer->color = {r, g, b, a};
	batch->buffer->uv = {uvs[4], uvs[5]};
	batch->buffer->texid = texSlot;
	batch->buffer++;

	batch->buffer->pos = {x + tex.width, y};
	batch->buffer->color = {r, g, b, a};
	batch->buffer->uv = {uvs[6], uvs[7]};
	batch->buffer->texid = texSlot;
	batch->buffer++;

	batch->indexcount += 6;
}

INTERNAL inline
void draw_texture(QuadBatch* batch, Texture tex, i32 x, i32 y) {
	draw_texture(batch, tex, x, y, 1, 1, 1, 1);
}

INTERNAL inline
void draw_texture(QuadBatch* batch, Texture tex, i32 x, i32 y, vec4 rgba) {
	draw_texture(batch, tex, x, y, rgba.x, rgba.y, rgba.z, rgba.w);
}

INTERNAL inline
void draw_texture_rotated(QuadBatch* batch, Texture tex, i32 x, i32 y, vec2 origin, f32 rotation, f32 r, f32 g, f32 b, f32 a) {
	if (tex.ID == 0)
		return;
	i32 texSlot = submit_tex(batch, tex);

	LOCAL f32 FLIP_VER_UVS[8] = { 0, 1, 0, 0, 1, 0, 1, 1 };
	LOCAL f32 FLIP_HOR_UVS[8] = { 1, 1, 1, 0, 0, 0, 0, 1 };
	LOCAL f32 DEFAULT_UVS[8] = { 0, 0, 0, 1, 1, 1, 1, 0 };
	LOCAL f32 FLIP_BOTH_UVS[8] = { 1, 0, 1, 1, 0, 1, 0, 0 };

	GLfloat* uvs;
	uvs = DEFAULT_UVS;
	if (tex.flip_flag & FLIP_HORIZONTAL && tex.flip_flag & FLIP_VERTICAL)
		uvs = FLIP_BOTH_UVS;
	if (tex.flip_flag & FLIP_HORIZONTAL)
		uvs = FLIP_HOR_UVS;
	if (tex.flip_flag & FLIP_VERTICAL)
		uvs = FLIP_VER_UVS;

	f32 cosine = 1;
	f32 sine = 0;
	if (rotation != 0) {
		rotation = deg_to_rad(rotation);
		cosine = cos(rotation);
		sine = sin(rotation);
	}

	batch->buffer->pos = {
		cosine * (x - origin.x) - sine * (y - origin.y) + origin.x, 
		sine * (x - origin.x) + cosine * (y - origin.y) + origin.y
	};
	batch->buffer->color = { r, g, b, a };
	batch->buffer->uv = { uvs[0], uvs[1] };
	batch->buffer->texid = texSlot;
	batch->buffer++;

	batch->buffer->pos = {
		cosine * (x - origin.x) - sine * ((y + tex.height) - origin.y) + origin.x,
		sine * (x - origin.x) + cosine * ((y + tex.height) - origin.y) + origin.y
	};
	batch->buffer->color = { r, g, b, a };
	batch->buffer->uv = { uvs[2], uvs[3] };
	batch->buffer->texid = texSlot;
	batch->buffer++;

	batch->buffer->pos = {
		cosine * ((x + tex.width) - origin.x) - sine * ((y + tex.height) - origin.y) + origin.x,
		sine * ((x + tex.width) - origin.x) + cosine * ((y + tex.height) - origin.y) + origin.y
	};
	batch->buffer->color = { r, g, b, a };
	batch->buffer->uv = { uvs[4], uvs[5] };
	batch->buffer->texid = texSlot;
	batch->buffer++;

	batch->buffer->pos = {
		cosine * ((x + tex.width) - origin.x) - sine * (y - origin.y) + origin.x,
		sine * ((x + tex.width) - origin.x) + cosine * (y - origin.y) + origin.y
	};
	batch->buffer->color = { r, g, b, a };
	batch->buffer->uv = { uvs[6], uvs[7] };
	batch->buffer->texid = texSlot;
	batch->buffer++;

	batch->indexcount += 6;
}

INTERNAL inline
void draw_texture_rotated(QuadBatch* batch, Texture tex, i32 x, i32 y, f32 rotateDegree, f32 r, f32 g, f32 b, f32 a) {
	draw_texture_rotated(batch, tex, x, y, V2(x + (tex.width / 2.0f), y + (tex.height / 2.0f)), rotateDegree, r, g, b, a);
}

INTERNAL inline
void draw_texture_rotated(QuadBatch* batch, Texture tex, i32 x, i32 y, f32 rotateDegree) {
	draw_texture_rotated(batch, tex, x, y, V2(x + (tex.width / 2.0f), y + (tex.height / 2.0f)), rotateDegree, 1, 1, 1, 1);
}

INTERNAL inline
void draw_texture_EX(QuadBatch* batch, Texture tex, Rect source, Rect dest, f32 r, f32 g, f32 b, f32 a) {
	if (tex.ID == 0)
		return;

	r /= 255.0f;
	g /= 255.0f;
	b /= 255.0f;
	a /= 255.0f;

	f32 uvs[8];
	if (tex.flip_flag == 0) {
		uvs[0] = source.x / tex.width;
		uvs[1] = source.y / tex.height;
		uvs[2] = source.x / tex.width;
		uvs[3] = (source.y + source.height) / tex.height;
		uvs[4] = (source.x + source.width) / tex.width;
		uvs[5] = (source.y + source.height) / tex.height;
		uvs[6] = (source.x + source.width) / tex.width;
		uvs[7] = source.y / tex.height;
	}
	else if (tex.flip_flag & FLIP_HORIZONTAL) {
		uvs[0] = (source.x + source.width) / tex.width;
		uvs[1] = source.y / tex.height;
		uvs[2] = (source.x + source.width) / tex.width;
		uvs[3] = (source.y + source.height) / tex.height;
		uvs[4] = source.x / tex.width;
		uvs[5] = (source.y + source.height) / tex.height;
		uvs[6] = source.x / tex.width;
		uvs[7] = source.y / tex.height;
	}
	else if (tex.flip_flag & FLIP_VERTICAL) {
		uvs[0] = source.x / tex.width;
		uvs[1] = (source.y + source.height) / tex.height;
		uvs[2] = source.x / tex.width;
		uvs[3] = source.y / tex.height;
		uvs[4] = (source.x + source.width) / tex.width;
		uvs[5] = source.y / tex.height;
		uvs[6] = (source.x + source.width) / tex.width;
		uvs[7] = (source.y + source.height) / tex.height;
	}
	else if (tex.flip_flag & FLIP_HORIZONTAL && tex.flip_flag & FLIP_VERTICAL) {
		uvs[0] = (source.x + source.width) / tex.width;
		uvs[1] = (source.y + source.height) / tex.height;
		uvs[2] = (source.x + source.width) / tex.width;
		uvs[3] = source.y / tex.height;
		uvs[4] = source.x / tex.width;
		uvs[5] = source.y / tex.height;
		uvs[6] = source.x / tex.width;
		uvs[7] = (source.y + source.height) / tex.height;
	}

	i32 texSlot = submit_tex(batch, tex);

	batch->buffer->pos = {dest.x, dest.y};
	batch->buffer->color = { r, g, b, a };
	batch->buffer->uv = { uvs[0], uvs[1] };
	batch->buffer->texid = texSlot;
	batch->buffer++;

	batch->buffer->pos = {dest.x, dest.y + dest.height};
	batch->buffer->color = { r, g, b, a };
	batch->buffer->uv = { uvs[2], uvs[3] };
	batch->buffer->texid = texSlot;
	batch->buffer++;
	
	batch->buffer->pos = {dest.x + dest.width, dest.y + dest.height};
	batch->buffer->color = { r, g, b, a };
	batch->buffer->uv = { uvs[4], uvs[5] };
	batch->buffer->texid = texSlot;
	batch->buffer++;
	
	batch->buffer->pos = {dest.x + dest.width, dest.y};
	batch->buffer->color = { r, g, b, a };
	batch->buffer->uv = { uvs[6], uvs[7] };
	batch->buffer->texid = texSlot;
	batch->buffer++;

	batch->indexcount += 6;
}

INTERNAL inline
void draw_texture_EX(QuadBatch* batch, Texture tex, Rect source, Rect dest) {
	draw_texture_EX(batch, tex, source, dest, 255.0f, 255.0f, 255.0f, 255.0f);
}

INTERNAL inline
void draw_texture_EX(QuadBatch* batch, Texture tex, Rect source, Rect dest, vec4 color) {
	draw_texture_EX(batch, tex, source, dest, color.x, color.y, color.z, color.w);
}

INTERNAL inline
void draw_framebuffer(QuadBatch* batch, Framebuffer buffer, i32 x, i32 y) {
	draw_texture(batch, buffer.texture, x, y);
}

INTERNAL inline
void draw_rectangle(QuadBatch* batch, i32 xPos, i32 yPos, i32 width, i32 height, f32 r, f32 g, f32 b, f32 a) {
	f32 x = (f32)xPos;
	f32 y = (f32)yPos;

	r /= 255;
	g /= 255;
	b /= 255;
	a /= 255;

	batch->buffer->pos = { x, y };
	batch->buffer->color = { r, g, b, a };
	batch->buffer->uv = { 0, 0 };
	batch->buffer->texid = 0;
	batch->buffer++;

	batch->buffer->pos = { x, y + height };
	batch->buffer->color = { r, g, b, a };
	batch->buffer->uv = { 0, 0 };
	batch->buffer->texid = 0;
	batch->buffer++;

	batch->buffer->pos = { x + width, y + height };
	batch->buffer->color = { r, g, b, a };
	batch->buffer->uv = { 0, 0 };
	batch->buffer->texid = 0;
	batch->buffer++;

	batch->buffer->pos = { x + width, y };
	batch->buffer->color = { r, g, b, a };
	batch->buffer->uv = { 0, 0 };
	batch->buffer->texid = 0;
	batch->buffer++;

	batch->indexcount += 6;
}

INTERNAL inline
void draw_rectangle(QuadBatch* batch, i32 x, i32 y, i32 width, i32 height, vec4 color) {
	draw_rectangle(batch, x, y, width, height, color.x, color.y, color.z, color.w);
}

//void draw_text(Font& font, const char* str, i32 xPos, i32 yPos, f32 r = 255.0f, f32 g = 255.0f, f32 b = 255.0f);

//void draw_text(Font& font, std::string str, i32 xPos, i32 yPos, f32 r = 255.0f, f32 g = 255.0f, f32 b = 255.0f);

INTERNAL inline
void unbind_quad_batch(QuadBatch* batch) {
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	for (u16 i = 0; i < batch->texcount; ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, batch->textures[i]);
	}

	glBindVertexArray(batch->vao);
	glEnableVertexAttribArray(0); //position
	glEnableVertexAttribArray(1); //color
	glEnableVertexAttribArray(2); //texture coordinates
	glEnableVertexAttribArray(3); //texture ID

	glDrawElements(GL_TRIANGLES, batch->indexcount, GL_UNSIGNED_SHORT, 0);

	glDisableVertexAttribArray(0); //position
	glDisableVertexAttribArray(1); //color
	glDisableVertexAttribArray(2); //texture coordinates
	glDisableVertexAttribArray(3); //textureID
	glBindVertexArray(0);

	for (u16 i = 0; i < batch->texcount; ++i)
		unbind_texture(batch->textures[i]);

	batch->indexcount = 0;
	batch->texcount = 0;

	stop_shader();
}

#include <string>
INTERNAL inline
Shader load_quad_shader() {
	LOCAL const GLchar* ORTHO_SHADER_FRAG_SHADER = R"FOO(
#version 330
out vec4 outColor;

in vec4 pass_color;
in vec2 pass_uv;
in float pass_texid;

uniform sampler2D sampler[32];

void main() {
	vec4 texColor = vec4(1.0);
	if(pass_texid > 0.0) {
		if(pass_texid == 1) texColor = texture(sampler[1], pass_uv);
		if(pass_texid == 2) texColor = texture(sampler[2], pass_uv);
		if(pass_texid == 3) texColor = texture(sampler[3], pass_uv);
		if(pass_texid == 4) texColor = texture(sampler[4], pass_uv);
		if(pass_texid == 5) texColor = texture(sampler[5], pass_uv);
		if(pass_texid == 6) texColor = texture(sampler[6], pass_uv);
		if(pass_texid == 7) texColor = texture(sampler[7], pass_uv);
		if(pass_texid == 8) texColor = texture(sampler[8], pass_uv);
		if(pass_texid == 9) texColor = texture(sampler[9], pass_uv);
		if(pass_texid == 10) texColor = texture(sampler[10], pass_uv);
		if(pass_texid == 11) texColor = texture(sampler[11], pass_uv);
		if(pass_texid == 12) texColor = texture(sampler[12], pass_uv);
		if(pass_texid == 13) texColor = texture(sampler[13], pass_uv);
		if(pass_texid == 14) texColor = texture(sampler[14], pass_uv);
		if(pass_texid == 15) texColor = texture(sampler[15], pass_uv);
		if(pass_texid == 16) texColor = texture(sampler[16], pass_uv);
		if(pass_texid == 17) texColor = texture(sampler[17], pass_uv);
		if(pass_texid == 18) texColor = texture(sampler[18], pass_uv);
		if(pass_texid == 19) texColor = texture(sampler[19], pass_uv);
		if(pass_texid == 20) texColor = texture(sampler[20], pass_uv);
		if(pass_texid == 21) texColor = texture(sampler[21], pass_uv);
		if(pass_texid == 22) texColor = texture(sampler[22], pass_uv);
		if(pass_texid == 23) texColor = texture(sampler[23], pass_uv);
		if(pass_texid == 24) texColor = texture(sampler[24], pass_uv);
		if(pass_texid == 25) texColor = texture(sampler[25], pass_uv);
		if(pass_texid == 26) texColor = texture(sampler[26], pass_uv);
		if(pass_texid == 27) texColor = texture(sampler[27], pass_uv);
		if(pass_texid == 28) texColor = texture(sampler[28], pass_uv);
		if(pass_texid == 29) texColor = texture(sampler[29], pass_uv);
		if(pass_texid == 30) texColor = texture(sampler[30], pass_uv);
		if(pass_texid == 31) texColor = texture(sampler[31], pass_uv);
	}
	outColor = pass_color * texColor;
}

)FOO";

	LOCAL const GLchar* ORTHO_SHADER_VERT_SHADER = R"FOO(
#version 330
in vec2 position;
in vec4 color;
in vec2 uv;
in float texid;

uniform mat4 projection = mat4(1.0);
uniform mat4 view = mat4(1.0);

out vec4 pass_color;
out vec2 pass_uv;
out float pass_texid;

void main() {
	pass_color = color;
	pass_uv = uv;
	pass_texid = texid;
	
	gl_Position = projection * view * vec4(position, 1.0, 1.0);
}

)FOO";
    Shader shader = load_shader_2D_from_strings(ORTHO_SHADER_VERT_SHADER, ORTHO_SHADER_FRAG_SHADER);
    start_shader(shader);
    for(int i = 0; i < 32; ++i) {
        std::string str = "textures[";
        str.append(std::to_string(i));
        str.append("]");
        upload_int(shader, str.c_str(), i);
    }
    stop_shader();
    return shader;
}

INTERNAL inline
void dispose_quad_batch(QuadBatch* batch) {
	glDeleteVertexArrays(1, &batch->vao);
	glDeleteBuffers(1, &batch->vbo);
	glDeleteBuffers(1, &batch->ebo);
	dispose_shader(batch->shader);
}

u32 inline rgba_to_u32(i32 r, i32 g, i32 b, i32 a) {
	return a << 24 | b << 16 | g << 8 | r;
}

#endif
