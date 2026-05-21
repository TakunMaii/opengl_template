#include "framebuffer.h"

#include "utils/log.h"
#include <glad/glad.h>
#include <string.h>

typedef struct {
    GLint internal_format;
    GLenum format;
    GLenum type;
    int channels;
} AttachmentFormatInfo;

static AttachmentFormatInfo get_attachment_format_info(FramebufferAttachmentFormat format)
{
    switch (format)
    {
    case FRAMEBUFFER_ATTACHMENT_RGBA8:
        return (AttachmentFormatInfo) {
            .internal_format = GL_RGBA8,
            .format = GL_RGBA,
            .type = GL_UNSIGNED_BYTE,
            .channels = 4,
        };
    case FRAMEBUFFER_ATTACHMENT_RGBA16F:
        return (AttachmentFormatInfo) {
            .internal_format = GL_RGBA16F,
            .format = GL_RGBA,
            .type = GL_FLOAT,
            .channels = 4,
        };
    case FRAMEBUFFER_ATTACHMENT_DEPTH24:
        return (AttachmentFormatInfo) {
            .internal_format = GL_DEPTH_COMPONENT24,
            .format = GL_DEPTH_COMPONENT,
            .type = GL_FLOAT,
            .channels = 1,
        };
    }

    PANIC("unsupported framebuffer attachment format");
    return (AttachmentFormatInfo){0};
}

static Texture create_attachment_texture(FramebufferAttachmentDesc desc, int width, int height)
{
    AttachmentFormatInfo info = get_attachment_format_info(desc.format);
    Texture texture = {0};

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glTexImage2D(GL_TEXTURE_2D, 0, info.internal_format, width, height, 0,
        info.format, info.type, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, desc.min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, desc.mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, desc.wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, desc.wrap_t);

    texture.width = width;
    texture.height = height;
    texture.channels = info.channels;
    return texture;
}

static FramebufferDesc framebuffer_desc_from_framebuffer(Framebuffer framebuffer, int width, int height)
{
    FramebufferDesc desc;

    memset(&desc, 0, sizeof(desc));
    desc.width = width;
    desc.height = height;
    desc.color_attachment_count = framebuffer.color_attachment_count;
    memcpy(desc.color_attachments, framebuffer.color_attachment_descs, sizeof(desc.color_attachments));
    desc.has_depth_attachment = framebuffer.has_depth_attachment;
    desc.depth_attachment = framebuffer.depth_attachment_desc;
    return desc;
}

FramebufferAttachmentDesc DefaultColorAttachmentDesc(FramebufferAttachmentFormat format)
{
    if (format == FRAMEBUFFER_ATTACHMENT_DEPTH24)
    {
        PANIC("depth format is not a color attachment");
    }

    return (FramebufferAttachmentDesc) {
        .format = format,
        .min_filter = GL_LINEAR,
        .mag_filter = GL_LINEAR,
        .wrap_s = GL_CLAMP_TO_EDGE,
        .wrap_t = GL_CLAMP_TO_EDGE,
    };
}

FramebufferAttachmentDesc DefaultDepthAttachmentDesc(void)
{
    return (FramebufferAttachmentDesc) {
        .format = FRAMEBUFFER_ATTACHMENT_DEPTH24,
        .min_filter = GL_NEAREST,
        .mag_filter = GL_NEAREST,
        .wrap_s = GL_CLAMP_TO_EDGE,
        .wrap_t = GL_CLAMP_TO_EDGE,
    };
}

FramebufferDesc DefaultFramebufferDesc(int width, int height)
{
    FramebufferDesc desc;

    memset(&desc, 0, sizeof(desc));
    desc.width = width;
    desc.height = height;
    return desc;
}

Framebuffer CreateFramebuffer(FramebufferDesc desc)
{
    Framebuffer framebuffer = {0};
    GLenum draw_buffers[FRAMEBUFFER_MAX_COLOR_ATTACHMENTS];
    GLenum status;
    size_t i;

    if (desc.width <= 0 || desc.height <= 0)
    {
        PANIC("framebuffer size must be positive");
    }
    if (desc.color_attachment_count > FRAMEBUFFER_MAX_COLOR_ATTACHMENTS)
    {
        PANIC("too many framebuffer color attachments");
    }

    framebuffer.width = desc.width;
    framebuffer.height = desc.height;
    framebuffer.color_attachment_count = desc.color_attachment_count;
    framebuffer.has_depth_attachment = desc.has_depth_attachment;
    memcpy(framebuffer.color_attachment_descs, desc.color_attachments, sizeof(framebuffer.color_attachment_descs));
    framebuffer.depth_attachment_desc = desc.depth_attachment;

    glGenFramebuffers(1, &framebuffer.id);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.id);

    for (i = 0; i < desc.color_attachment_count; ++i)
    {
        framebuffer.color_attachments[i] = create_attachment_texture(
            desc.color_attachments[i], desc.width, desc.height);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (GLenum)i,
            GL_TEXTURE_2D, framebuffer.color_attachments[i].id, 0);
        draw_buffers[i] = GL_COLOR_ATTACHMENT0 + (GLenum)i;
    }

    if (desc.color_attachment_count > 0)
    {
        glDrawBuffers((GLsizei)desc.color_attachment_count, draw_buffers);
    }
    else
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    if (desc.has_depth_attachment)
    {
        framebuffer.depth_attachment = create_attachment_texture(
            desc.depth_attachment, desc.width, desc.height);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
            framebuffer.depth_attachment.id, 0);
    }

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        ReleaseFramebuffer(framebuffer);
        PANIC("framebuffer incomplete");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return framebuffer;
}

void BindFramebuffer(Framebuffer framebuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.id);
}

void BindDefaultFramebuffer(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ResizeFramebuffer(Framebuffer* framebuffer, int width, int height)
{
    FramebufferDesc desc;

    if (framebuffer == NULL)
    {
        PANIC("framebuffer must not be null");
    }
    if (width <= 0 || height <= 0)
    {
        PANIC("framebuffer size must be positive");
    }
    if (framebuffer->width == width && framebuffer->height == height)
    {
        return;
    }

    desc = framebuffer_desc_from_framebuffer(*framebuffer, width, height);
    ReleaseFramebuffer(*framebuffer);
    *framebuffer = CreateFramebuffer(desc);
}

Texture GetFramebufferColorAttachment(Framebuffer framebuffer, size_t index)
{
    if (index >= framebuffer.color_attachment_count)
    {
        PANIC("framebuffer color attachment index out of range");
    }

    return framebuffer.color_attachments[index];
}

Texture GetFramebufferDepthAttachment(Framebuffer framebuffer)
{
    if (!framebuffer.has_depth_attachment)
    {
        PANIC("framebuffer has no depth attachment");
    }

    return framebuffer.depth_attachment;
}

void ReleaseFramebuffer(Framebuffer framebuffer)
{
    size_t i;

    if (framebuffer.id != 0)
    {
        glDeleteFramebuffers(1, &framebuffer.id);
    }

    for (i = 0; i < framebuffer.color_attachment_count; ++i)
    {
        if (framebuffer.color_attachments[i].id != 0)
        {
            glDeleteTextures(1, &framebuffer.color_attachments[i].id);
        }
    }

    if (framebuffer.has_depth_attachment && framebuffer.depth_attachment.id != 0)
    {
        glDeleteTextures(1, &framebuffer.depth_attachment.id);
    }
}
