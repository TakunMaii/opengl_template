#ifndef TEXTURE_H
#define TEXTURE_H

#include "glad/glad.h"

typedef struct {
    GLuint id;
    int width;
    int height;
    int channels;
} Texture;

typedef struct {
    GLint wrap_s;
    GLint wrap_t;
    GLint min_filter;
    GLint mag_filter;
    int flip_y;
    int generate_mipmap;
} TextureLoadOptions;

TextureLoadOptions DefaultTextureLoadOptions(void);
Texture LoadTexture(const char* image_path);
Texture LoadTextureEx(const char* image_path, TextureLoadOptions options);
void BindTexture(Texture texture, unsigned int unit);
void UnbindTexture(unsigned int unit);
void ReleaseTexture(Texture texture);

#endif /* TEXTURE_H */
