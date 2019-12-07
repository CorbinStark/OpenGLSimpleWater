///////////////////////////////////////////////////////////////////////////
// FILE:                       texture.h                                 //
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

#ifndef TEXTURE_H
#define TEXTURE_H

#include "defines.h"
#include <vector>
#include <SOIL.h>

GLOBAL const u16 FLIP_HORIZONTAL = 1;
GLOBAL const u16 FLIP_VERTICAL = 2;

struct Texture {
    GLuint ID;
    u64 flip_flag;
    i32 width;
    i32 height;
};

INTERNAL inline
Texture create_blank_texture(u32 width = 0, u32 height = 0) {
    Texture texture;
    glGenTextures(1, &texture.ID);
    glBindTexture(GL_TEXTURE_2D, texture.ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    texture.width = width;
    texture.height = height;
    texture.flip_flag = 0;

    return texture;
}

INTERNAL inline
Texture load_texture(unsigned char* pixels, u32 width, u32 height, u16 param) {
    Texture texture;
    texture.width = width;
    texture.height = height;

    glGenTextures(1, &texture.ID);
    glBindTexture(GL_TEXTURE_2D, texture.ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, param);
    glBindTexture(GL_TEXTURE_2D, 0);
    texture.flip_flag = 0;

    return texture;
}

INTERNAL inline
void dispose_texture(Texture& texture) {
    glDeleteTextures(1, &texture.ID);
    texture.ID = 0;
}

INTERNAL inline
Texture load_texture(const char* filepath, u16 param) {
    Texture texture;
    glGenTextures(1, &texture.ID);
    glBindTexture(GL_TEXTURE_2D, texture.ID);
    unsigned char* image = SOIL_load_image(filepath, &texture.width, &texture.height, 0, SOIL_LOAD_RGBA);
    if (image != NULL) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    }
    else {
        BMT_LOG(WARNING, "[%s] Texture could not be loaded! Disposing and returning blank texture.", filepath);
        dispose_texture(texture);
    }
    SOIL_free_image_data(image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, param);
    glBindTexture(GL_TEXTURE_2D, 0);
    texture.flip_flag = 0;

    return texture;
}

INTERNAL inline
void set_texture_pixels(Texture texture, unsigned char* pixels, u32 width, u32 height) {
    glBindTexture(GL_TEXTURE_2D, texture.ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
}

INTERNAL inline
void set_texture_pixels_from_file(Texture texture, const char* filepath) {
    glBindTexture(GL_TEXTURE_2D, texture.ID);
    unsigned char* image = SOIL_load_image(filepath, &texture.width, &texture.height, 0, SOIL_LOAD_RGBA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);
}

INTERNAL inline
void bind_texture(Texture texture, u32 slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture.ID);
}

INTERNAL inline
void unbind_texture(u32 slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, 0);
}

//==========================================================================================
//Description: Sets the wrap_x of the texture (horizontal wrapping)
//
//Parameters: 
//		-A texture to set parameter of
//		-The type of wrapping to do
//
//Comments: Unbinds whatever is in slot 0
//==========================================================================================
INTERNAL inline
void set_texture_wrap_x(Texture texture, u32 type) {
    bind_texture(texture, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, type);
    unbind_texture(0);
}

//==========================================================================================
//Description: Sets the wrap_y of the texture (vertical wrapping)
//
//Parameters: 
//		-A texture to set parameter of
//		-The type of wrapping to do
//
//Comments: Unbinds whatever is in slot 0
//==========================================================================================
INTERNAL inline
void set_texture_wrap_y(Texture texture, u32 type) {
    bind_texture(texture, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, type);
    unbind_texture(0);
}

struct Framebuffer {
    GLuint ID;
    Texture texture;
};

#define DEPTHBUFFER 0
#define COLORBUFFER 1

INTERNAL inline
Framebuffer create_framebuffer(u32 width, u32 height, u16 param, u8 buffertype) {
    assert(buffertype < 2);

    Framebuffer buffer;
    buffer.texture.width = width;
    buffer.texture.height = height;
    buffer.texture.flip_flag = 0;

    glGenTextures(1, &buffer.texture.ID);
    glBindTexture(GL_TEXTURE_2D, buffer.texture.ID);
    if (buffertype == COLORBUFFER) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    }
    else if (buffertype == DEPTHBUFFER) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, param);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &buffer.ID);
    glBindFramebuffer(GL_FRAMEBUFFER, buffer.ID);

    if (buffertype == COLORBUFFER)
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer.texture.ID, 0);
    else if (buffertype == DEPTHBUFFER) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer.texture.ID, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    GLuint depthBuffer;
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
        BMT_LOG(INFO, "Framebuffer #%d successfully created", buffer.ID);
    }
    else {
        BMT_LOG(WARNING, "Framebuffer #%d not complete!", buffer.ID);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return buffer;
}

INTERNAL inline
Framebuffer create_color_buffer(u32 width, u32 height, u16 param) {
    return create_framebuffer(width, height, param, COLORBUFFER);
}

INTERNAL inline
Framebuffer create_depth_buffer(u32 width, u32 height, u16 param) {
    return create_framebuffer(width, height, param, DEPTHBUFFER);
}

INTERNAL inline
void dispose_framebuffer(Framebuffer buffer) {
    dispose_texture(buffer.texture);
    glDeleteFramebuffers(1, &buffer.ID);
}

INTERNAL inline
void bind_framebuffer(Framebuffer buffer) {
    glBindFramebuffer(GL_FRAMEBUFFER, buffer.ID);
}

INTERNAL inline
void unbind_framebuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//==========================================================================================
//Description: Clears the bound framebuffer (by default the window)
//
//Comments: The window is cleared by end_drawing() so no need to do it yourself.
//			If you want to clear the screen mid-frame for some reason, this will do that.
//==========================================================================================
INTERNAL inline
void clear_bound_framebuffer() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

#include <mutex>
#include <thread>

std::mutex bmtTexMutex;
static i32 bmtTexCurrRow;
const int BYTES_PER_PIXEL = 3;

INTERNAL inline
vec3 get_new_rgb(const unsigned char* src, i32 x, i32 y, size_t src_width, vec2 scaleFactor) {
    i32 srcX = x / scaleFactor.x;
    i32 srcY = y / scaleFactor.y;

    i32 index;
    index = srcX + srcY * src_width;
    index *= BYTES_PER_PIXEL;
    vec3 rgb00 = { (f32)src[index + 2], (f32)src[index + 1], (f32)src[index + 0] };

    index = srcX + (srcY + 1) * src_width;
    index *= BYTES_PER_PIXEL;
    vec3 rgb01 = { (f32)src[index + 2], (f32)src[index + 1], (f32)src[index + 0] };

    index = (srcX + 1) + srcY * src_width;
    index *= BYTES_PER_PIXEL;
    vec3 rgb10 = { (f32)src[index + 2], (f32)src[index + 1], (f32)src[index + 0] };

    index = (srcX + 1) + (srcY + 1) * src_width;
    index *= BYTES_PER_PIXEL;
    vec3 rgb11 = { (f32)src[index + 2], (f32)src[index + 1], (f32)src[index + 0] };

    f32 xDistPercent = (x - ((srcX+1) * scaleFactor.x)) / (((srcX+0) * scaleFactor.x) - ((srcX+1) * scaleFactor.x));

    vec3 newRGB00 = {
        xDistPercent * rgb00.x + (1-xDistPercent) * rgb10.x,
        xDistPercent * rgb00.y + (1-xDistPercent) * rgb10.y,
        xDistPercent * rgb00.z + (1-xDistPercent) * rgb10.z
    };
    vec3 newRGB01 = {
        xDistPercent * rgb01.x + (1-xDistPercent) * rgb11.x,
        xDistPercent * rgb01.y + (1-xDistPercent) * rgb11.y,
        xDistPercent * rgb01.z + (1-xDistPercent) * rgb11.z
    };
    f32 yDistPercent = (y - ((srcY+1) * scaleFactor.y)) / (((srcY+0) * scaleFactor.y) - ((srcY+1) * scaleFactor.y));

    vec3 newRGB = {
        yDistPercent * newRGB00.x + (1-yDistPercent) * newRGB01.x,
        yDistPercent * newRGB00.y + (1-yDistPercent) * newRGB01.y,
        yDistPercent * newRGB00.z + (1-yDistPercent) * newRGB01.z
    };

    return newRGB;
}

INTERNAL inline
void interpolate(const unsigned char* src, unsigned char* dest, size_t dest_width, size_t dest_height, size_t src_width, size_t src_height) {
    vec2 scaleFactor = { (f32)dest_width / src_width , (f32)dest_height / src_height };

    i32 index = 0;
    while (bmtTexCurrRow < dest_height) {
        bmtTexMutex.lock();
        i32 y = bmtTexCurrRow;
        bmtTexCurrRow++;
        bmtTexMutex.unlock();

        for (i32 x = 0; x < dest_width; ++x) {

            vec3 RGB = get_new_rgb(src, x, y, src_width, scaleFactor);

            i32 index = x + y * dest_width;
            i32 offset = index * BYTES_PER_PIXEL;

            dest[offset + 0] = RGB.z;
            dest[offset + 1] = RGB.y;
            dest[offset + 2] = RGB.x;
        }
    }
}

INTERNAL inline
void scale_image_linear(int num_threads, size_t src_width, size_t src_height, const unsigned char* source, size_t dest_width, size_t dest_height, unsigned char* dest) {
    std::vector<std::thread*> threadpool;

    bmtTexMutex.lock();
    bmtTexCurrRow = 0;
    bmtTexMutex.unlock();

    for (i32 i = 0; i < num_threads; ++i) {
        std::thread* thread = new std::thread(interpolate, source, dest, dest_width, dest_height, src_width, src_height);
        threadpool.push_back(thread);
    }
    for (i32 i = 0; i < num_threads; ++i) {
        threadpool.at(i)->join();
        delete threadpool.at(i);
    }
}

#endif
