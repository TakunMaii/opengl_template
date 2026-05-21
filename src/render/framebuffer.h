#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "texture.h"
#include <stdbool.h>
#include <stddef.h>

#define FRAMEBUFFER_MAX_COLOR_ATTACHMENTS 4

typedef enum {
    FRAMEBUFFER_ATTACHMENT_RGBA8,
    FRAMEBUFFER_ATTACHMENT_RGBA16F,
    FRAMEBUFFER_ATTACHMENT_DEPTH24
} FramebufferAttachmentFormat;

typedef struct {
    FramebufferAttachmentFormat format;
    GLint min_filter;
    GLint mag_filter;
    GLint wrap_s;
    GLint wrap_t;
} FramebufferAttachmentDesc;

typedef struct {
    int width;
    int height;
    size_t color_attachment_count;
    FramebufferAttachmentDesc color_attachments[FRAMEBUFFER_MAX_COLOR_ATTACHMENTS];
    bool has_depth_attachment;
    FramebufferAttachmentDesc depth_attachment;
} FramebufferDesc;

typedef struct {
    GLuint id;
    Texture color_attachments[FRAMEBUFFER_MAX_COLOR_ATTACHMENTS];
    size_t color_attachment_count;
    Texture depth_attachment;
    bool has_depth_attachment;
    int width;
    int height;
    FramebufferAttachmentDesc color_attachment_descs[FRAMEBUFFER_MAX_COLOR_ATTACHMENTS];
    FramebufferAttachmentDesc depth_attachment_desc;
} Framebuffer;

FramebufferAttachmentDesc DefaultColorAttachmentDesc(FramebufferAttachmentFormat format);
FramebufferAttachmentDesc DefaultDepthAttachmentDesc(void);
FramebufferDesc DefaultFramebufferDesc(int width, int height);

Framebuffer CreateFramebuffer(FramebufferDesc desc);
void BindFramebuffer(Framebuffer framebuffer);
void BindDefaultFramebuffer(void);
void ResizeFramebuffer(Framebuffer* framebuffer, int width, int height);
Texture GetFramebufferColorAttachment(Framebuffer framebuffer, size_t index);
Texture GetFramebufferDepthAttachment(Framebuffer framebuffer);
void ReleaseFramebuffer(Framebuffer framebuffer);

#endif /* FRAMEBUFFER_H */
