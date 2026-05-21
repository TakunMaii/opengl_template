#ifndef RENDERER_H
#define RENDERER_H

#include "math/linear_algebra.h"
#include "render/framebuffer.h"
#include "render/mesh.h"
#include "render/texture.h"
#include "scene/scene.h"
#include "shader/shader.h"
#include <stdbool.h>

typedef enum {
    RENDER_MODE_PBR = 0,
    RENDER_MODE_TOON = 1
} RenderMode;

typedef struct {
    Mat4 view;
    Mat4 projection;
    Vec3 position;
} Camera3D;

typedef struct {
    Vec3 light_direction;
    Vec3 ambient_color;
    Vec4 clear_color;
    Vec4 outline_color;
    float normal_edge_threshold;
    float depth_edge_threshold;
    float outline_strength;
    RenderMode render_mode;
    bool enable_outline;
} RendererSettings;

typedef struct {
    Shader scene_shader;
    Shader composite_shader;
    Mesh screen_quad;
    Framebuffer framebuffer;
    Texture white_texture;
    Texture normal_texture;
    Texture black_texture;
    RendererSettings settings;
} Renderer;

RendererSettings DefaultRendererSettings(void);
Renderer CreateRenderer(int width, int height, RendererSettings settings);
void ResizeRenderer(Renderer* renderer, int width, int height);
void RenderScene(Renderer* renderer, const Scene* scene, Camera3D camera);
void ReleaseRenderer(Renderer renderer);

#endif /* RENDERER_H */
