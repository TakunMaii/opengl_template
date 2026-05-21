#define STB_IMAGE_IMPLEMENTATION
#include "texture.h"

#include "stb_image.h"
#include "utils/log.h"

static GLenum texture_format_from_channels(int channels)
{
    switch (channels)
    {
        case 1:
            return GL_RED;
        case 2:
            return GL_RG;
        case 3:
            return GL_RGB;
        case 4:
            return GL_RGBA;
        default:
            PANIC("unsupported texture channel count");
            return GL_RGB;
    }
}

TextureLoadOptions DefaultTextureLoadOptions(void)
{
    return (TextureLoadOptions) {
        .wrap_s = GL_REPEAT,
        .wrap_t = GL_REPEAT,
        .min_filter = GL_LINEAR_MIPMAP_LINEAR,
        .mag_filter = GL_LINEAR,
        .flip_y = 1,
        .generate_mipmap = 1,
    };
}

Texture LoadTexture(const char* image_path)
{
    return LoadTextureEx(image_path, DefaultTextureLoadOptions());
}

Texture LoadTextureEx(const char* image_path, TextureLoadOptions options)
{
    Texture texture = {0};
    unsigned char* pixels;
    GLenum format;

    stbi_set_flip_vertically_on_load(options.flip_y);
    pixels = stbi_load(image_path, &texture.width, &texture.height, &texture.channels, 0);
    if (pixels == NULL)
    {
        PANIC(stbi_failure_reason());
    }

    format = texture_format_from_channels(texture.channels);

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, options.wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, options.wrap_t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options.min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options.mag_filter);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, (GLint)format, texture.width, texture.height, 0, format, GL_UNSIGNED_BYTE, pixels);

    if (options.generate_mipmap)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(pixels);

    return texture;
}

void BindTexture(Texture texture, unsigned int unit)
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture.id);
}

void UnbindTexture(unsigned int unit)
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ReleaseTexture(Texture texture)
{
    glDeleteTextures(1, &texture.id);
}
