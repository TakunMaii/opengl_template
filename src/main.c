#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <glad/glad.h>
#include "core/core.h"
#include "core/input.h"
#include "math/linear_algebra.h"
#include "render/color.h"
#include "render/framebuffer.h"
#include "render/mesh.h"
#include "render/model.h"
#include "render/texture.h"
#include "shader/shader.h"
#include "utils/log.h"

#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

typedef struct {
    float shadow_threshold;
    float shadow_softness;
    float shadow_strength;
    Vec3 shadow_tint;
    float highlight_threshold;
    float highlight_softness;
    float highlight_strength;
    Vec3 highlight_color;
    float rim_threshold;
    float rim_softness;
    float rim_strength;
    Vec3 rim_color;
    Vec3 ambient_color;
} CharacterPartStyle;

static Vec3 camera_front_from_angles(float yaw_degrees, float pitch_degrees)
{
    float yaw = radiansf(yaw_degrees);
    float pitch = radiansf(pitch_degrees);

    return normalize3(vec3(
        cosf(yaw) * cosf(pitch),
        sinf(pitch),
        sinf(yaw) * cosf(pitch)
    ));
}

static bool string_contains(const char* text, const char* token)
{
    return strstr(text, token) != NULL;
}

static CharacterPartStyle default_character_style(void)
{
    return (CharacterPartStyle) {
        .shadow_threshold = 0.58f,
        .shadow_softness = 0.045f,
        .shadow_strength = 0.72f,
        .shadow_tint = vec3(0.66f, 0.74f, 0.90f),
        .highlight_threshold = 0.72f,
        .highlight_softness = 0.04f,
        .highlight_strength = 0.07f,
        .highlight_color = vec3(1.0f, 0.97f, 0.93f),
        .rim_threshold = 0.62f,
        .rim_softness = 0.08f,
        .rim_strength = 0.12f,
        .rim_color = vec3(0.95f, 0.97f, 1.0f),
        .ambient_color = vec3(0.08f, 0.09f, 0.11f),
    };
}

static CharacterPartStyle style_for_mesh_name(const char* mesh_name)
{
    CharacterPartStyle style = default_character_style();

    if (string_contains(mesh_name, "Hair"))
    {
        style.shadow_threshold = 0.60f;
        style.shadow_strength = 0.68f;
        style.highlight_threshold = 0.60f;
        style.highlight_softness = 0.08f;
        style.highlight_strength = 0.20f;
        style.highlight_color = vec3(1.0f, 0.95f, 0.88f);
        style.rim_threshold = 0.56f;
        style.rim_strength = 0.20f;
        style.rim_color = vec3(0.82f, 0.90f, 1.0f);
    }
    else if (string_contains(mesh_name, "Skin"))
    {
        style.shadow_threshold = 0.52f;
        style.shadow_softness = 0.06f;
        style.shadow_strength = 0.48f;
        style.shadow_tint = vec3(0.90f, 0.78f, 0.76f);
        style.highlight_threshold = 0.80f;
        style.highlight_strength = 0.03f;
        style.rim_threshold = 0.68f;
        style.rim_strength = 0.08f;
        style.rim_color = vec3(1.0f, 0.86f, 0.83f);
        style.ambient_color = vec3(0.10f, 0.08f, 0.08f);
    }
    else if (string_contains(mesh_name, "Shoes") ||
        string_contains(mesh_name, "acc") ||
        string_contains(mesh_name, "accessories"))
    {
        style.shadow_threshold = 0.63f;
        style.shadow_strength = 0.82f;
        style.highlight_threshold = 0.56f;
        style.highlight_softness = 0.05f;
        style.highlight_strength = 0.18f;
        style.rim_threshold = 0.58f;
        style.rim_strength = 0.16f;
    }
    else if (string_contains(mesh_name, "jacket") || string_contains(mesh_name, "inner"))
    {
        style.shadow_threshold = 0.57f;
        style.shadow_strength = 0.74f;
        style.highlight_threshold = 0.70f;
        style.highlight_strength = 0.06f;
        style.rim_threshold = 0.60f;
        style.rim_strength = 0.10f;
    }

    return style;
}

static void set_body_shader_style_uniforms(Shader shader, CharacterPartStyle style)
{
    SetShaderFloat(shader, "uShadowThreshold", style.shadow_threshold);
    SetShaderFloat(shader, "uShadowSoftness", style.shadow_softness);
    SetShaderFloat(shader, "uShadowStrength", style.shadow_strength);
    SetShaderVec3(shader, "uShadowTint", style.shadow_tint);
    SetShaderFloat(shader, "uHighlightThreshold", style.highlight_threshold);
    SetShaderFloat(shader, "uHighlightSoftness", style.highlight_softness);
    SetShaderFloat(shader, "uHighlightStrength", style.highlight_strength);
    SetShaderVec3(shader, "uHighlightColor", style.highlight_color);
    SetShaderFloat(shader, "uRimThreshold", style.rim_threshold);
    SetShaderFloat(shader, "uRimSoftness", style.rim_softness);
    SetShaderFloat(shader, "uRimStrength", style.rim_strength);
    SetShaderVec3(shader, "uRimColor", style.rim_color);
    SetShaderVec3(shader, "uAmbientColor", style.ambient_color);
}

int main(int argc, char **argv)
{
    static const char* texture_paths[] = {
        "assets/textures/Image_0.png",
        "assets/textures/Image_1.png",
        "assets/textures/Image_2.png",
        "assets/textures/Image_3.png",
        "assets/textures/Image_4.png",
        "assets/textures/Image_5.png",
        "assets/textures/Image_6.png",
    };

    Model test_model;
    Shader body_shader;
    Shader composite_shader;
    Texture mesh_textures[ARRAY_COUNT(texture_paths)];
    Mesh screen_quad;
    Framebuffer scene_framebuffer;
    FramebufferDesc scene_framebuffer_desc;
    Vec3 camera_position = vec3(0.0f, 0.09f, 0.35f);
    Vec3 world_up = vec3(0.0f, 1.0f, 0.0f);
    Vec3 light_direction = normalize3(vec3(-0.45f, -1.0f, -0.25f));
    float yaw = -90.0f;
    float pitch = 0.0f;
    float move_speed = 0.25f;
    float mouse_sensitivity = 0.08f;
    double last_frame_time = GetTime();
    Mat4 model_matrix = mat4_identity();
    int framebuffer_width;
    int framebuffer_height;
    size_t texture_index;

    (void)argc;
    (void)argv;

    InitWindow(800, 600, "OpenGl template");
    InitInput();

    test_model = LoadModelFromObj("assets/character.obj");
    body_shader = LoadShader("assets/shaders/anime_body.vs", "assets/shaders/anime_body.fs");
    composite_shader = LoadShader("assets/shaders/postprocess_outline.vs", "assets/shaders/postprocess_outline.fs");
    screen_quad = GenerateRectangle2D(vec2(0.0f, 0.0f), vec2(2.0f, 2.0f), color(1.0f, 1.0f, 1.0f, 1.0f));

    if (test_model.mesh_count != ARRAY_COUNT(texture_paths))
    {
        PANIC("model mesh count and texture count mismatch");
    }

    for (texture_index = 0; texture_index < ARRAY_COUNT(texture_paths); ++texture_index)
    {
        mesh_textures[texture_index] = LoadTexture(texture_paths[texture_index]);
    }

    GetFramebufferSize(&framebuffer_width, &framebuffer_height);
    if (framebuffer_width <= 0)
    {
        framebuffer_width = 1;
    }
    if (framebuffer_height <= 0)
    {
        framebuffer_height = 1;
    }
    scene_framebuffer_desc = DefaultFramebufferDesc(framebuffer_width, framebuffer_height);
    scene_framebuffer_desc.color_attachment_count = 2;
    scene_framebuffer_desc.color_attachments[0] = DefaultColorAttachmentDesc(FRAMEBUFFER_ATTACHMENT_RGBA8);
    scene_framebuffer_desc.color_attachments[1] = DefaultColorAttachmentDesc(FRAMEBUFFER_ATTACHMENT_RGBA8);
    scene_framebuffer_desc.has_depth_attachment = true;
    scene_framebuffer_desc.depth_attachment = DefaultDepthAttachmentDesc();
    scene_framebuffer = CreateFramebuffer(scene_framebuffer_desc);

    SetCursorCaptured(true);

    while (!WindowShouldClose())
    {
        double current_frame_time = GetTime();
        float delta_time = (float)(current_frame_time - last_frame_time);
        Vec2 mouse_delta;
        Vec3 camera_front;
        Vec3 camera_right;
        Vec3 camera_up;
        Vec3 move_forward;
        Mat4 projection;
        Mat4 view;
        size_t mesh_index;

        last_frame_time = current_frame_time;
        UpdateInput();

        if (IsKeyPressed(KEY_ESCAPE))
        {
            SetCursorCaptured(!IsCursorCaptured());
        }
        else if (!IsCursorCaptured() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            SetCursorCaptured(true);
        }

        mouse_delta = GetMouseDelta();
        if (IsCursorCaptured())
        {
            yaw += mouse_delta.x * mouse_sensitivity;
            pitch -= mouse_delta.y * mouse_sensitivity;
            pitch = clampf(pitch, -89.0f, 89.0f);
        }

        camera_front = camera_front_from_angles(yaw, pitch);
        camera_right = normalize3(cross(camera_front, world_up));
        camera_up = normalize3(cross(camera_right, camera_front));
        move_forward = normalize3(vec3(camera_front.x, 0.0f, camera_front.z));

        if (IsKeyDown(KEY_W))
        {
            camera_position = add3(camera_position, scale3(move_forward, move_speed * delta_time));
        }
        if (IsKeyDown(KEY_S))
        {
            camera_position = sub3(camera_position, scale3(move_forward, move_speed * delta_time));
        }
        if (IsKeyDown(KEY_A))
        {
            camera_position = sub3(camera_position, scale3(camera_right, move_speed * delta_time));
        }
        if (IsKeyDown(KEY_D))
        {
            camera_position = add3(camera_position, scale3(camera_right, move_speed * delta_time));
        }
        if (IsKeyDown(KEY_SPACE))
        {
            camera_position = add3(camera_position, scale3(world_up, move_speed * delta_time));
        }
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
        {
            camera_position = sub3(camera_position, scale3(world_up, move_speed * delta_time));
        }

        GetFramebufferSize(&framebuffer_width, &framebuffer_height);
        if (framebuffer_width <= 0)
        {
            framebuffer_width = 1;
        }
        if (framebuffer_height <= 0)
        {
            framebuffer_height = 1;
        }

        ResizeFramebuffer(&scene_framebuffer, framebuffer_width, framebuffer_height);

        projection = mat4_perspective(
            radiansf(45.0f),
            (float)framebuffer_width / (float)framebuffer_height,
            0.01f,
            10.0f
        );
        view = mat4_look_at(camera_position, add3(camera_position, camera_front), camera_up);

        BeginDrawing();

        BindFramebuffer(scene_framebuffer);
        glViewport(0, 0, framebuffer_width, framebuffer_height);
        glClearColor(0.87f, 0.92f, 0.98f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        BeginShaderMode(body_shader);
        SetShaderMat4(body_shader, "uModel", model_matrix);
        SetShaderMat4(body_shader, "uView", view);
        SetShaderMat4(body_shader, "uProjection", projection);
        SetShaderVec3(body_shader, "uViewPosition", camera_position);
        SetShaderVec3(body_shader, "uLightDirection", light_direction);
        SetShaderInt(body_shader, "uDiffuseMap", 0);
        for (mesh_index = 0; mesh_index < test_model.mesh_count; ++mesh_index)
        {
            CharacterPartStyle style;
            ModelMesh mesh = GetModelMesh(test_model, mesh_index);

            style = style_for_mesh_name(mesh.name);
            set_body_shader_style_uniforms(body_shader, style);
            BindTexture(mesh_textures[mesh_index], 0);
            RenderMesh(mesh.mesh);
            UnbindTexture(0);
        }
        EndShaderMode();

        BindDefaultFramebuffer();
        glViewport(0, 0, framebuffer_width, framebuffer_height);
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.87f, 0.92f, 0.98f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        BeginShaderMode(composite_shader);
        SetShaderInt(composite_shader, "uSceneColor", 0);
        SetShaderInt(composite_shader, "uSceneNormal", 1);
        SetShaderInt(composite_shader, "uSceneDepth", 2);
        SetShaderVec2(
            composite_shader,
            "uTexelSize",
            vec2(1.0f / (float)framebuffer_width, 1.0f / (float)framebuffer_height)
        );
        SetShaderVec4(composite_shader, "uOutlineColor", vec4(0.08f, 0.09f, 0.13f, 1.0f));
        SetShaderFloat(composite_shader, "uNormalEdgeThreshold", 0.18f);
        SetShaderFloat(composite_shader, "uDepthEdgeThreshold", 0.0035f);
        SetShaderFloat(composite_shader, "uOutlineStrength", 1.0f);
        BindTexture(GetFramebufferColorAttachment(scene_framebuffer, 0), 0);
        BindTexture(GetFramebufferColorAttachment(scene_framebuffer, 1), 1);
        BindTexture(GetFramebufferDepthAttachment(scene_framebuffer), 2);
        RenderMesh(screen_quad);
        UnbindTexture(2);
        UnbindTexture(1);
        UnbindTexture(0);
        EndShaderMode();

        glEnable(GL_DEPTH_TEST);

        EndDrawing();
    }

    ReleaseFramebuffer(scene_framebuffer);
    ReleaseMesh(screen_quad);
    ReleaseModel(test_model);
    ReleaseShader(body_shader);
    ReleaseShader(composite_shader);
    for (texture_index = 0; texture_index < ARRAY_COUNT(texture_paths); ++texture_index)
    {
        ReleaseTexture(mesh_textures[texture_index]);
    }

    CloseWindow();
    return 0;
}
