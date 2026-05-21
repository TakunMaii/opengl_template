#include <stdbool.h>
#include <math.h>
#include "core/core.h"
#include "core/input.h"
#include "math/linear_algebra.h"
#include "render/model.h"
#include "render/texture.h"
#include "shader/shader.h"
#include "utils/log.h"

#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

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

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    static const char* texture_paths[] = {
        "assets/textures/Image_0.png",
        "assets/textures/Image_1.png",
        "assets/textures/Image_2.png",
        "assets/textures/Image_3.png",
        "assets/textures/Image_4.png",
        "assets/textures/Image_5.png",
        "assets/textures/Image_6.png",
    };

    InitWindow(800, 600, "OpenGl template");
    InitInput();
    Model test_model = LoadModelFromObj("assets/character.obj");
    Shader test_shader = LoadShader("assets/shaders/test.vs", "assets/shaders/test.fs");
    Texture mesh_textures[ARRAY_COUNT(texture_paths)];
    Vec3 camera_position = vec3(0.0f, 0.09f, 0.35f);
    Vec3 world_up = vec3(0.0f, 1.0f, 0.0f);
    float yaw = -90.0f;
    float pitch = 0.0f;
    float move_speed = 0.25f;
    float mouse_sensitivity = 0.08f;
    double last_frame_time = GetTime();
    size_t texture_index;

    if (test_model.mesh_count != ARRAY_COUNT(texture_paths))
    {
        PANIC("model mesh count and texture count mismatch");
    }

    for (texture_index = 0; texture_index < ARRAY_COUNT(texture_paths); ++texture_index)
    {
        mesh_textures[texture_index] = LoadTexture(texture_paths[texture_index]);
    }

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
        int framebuffer_width;
        int framebuffer_height;
        size_t mesh_index;

        last_frame_time = current_frame_time;
        UpdateInput();

        if (IsKeyPressed(KEY_ESCAPE))
        {
            SetCursorCaptured(!IsCursorCaptured());
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
        projection = mat4_perspective(
            radiansf(45.0f),
            (float)framebuffer_width / (float)framebuffer_height,
            0.01f,
            10.0f
        );
        view = mat4_look_at(camera_position, add3(camera_position, camera_front), camera_up);

        ClearBackground(color(0.3f, 0.4f, 0.5f, 1.0f));

        BeginDrawing();
        BeginShaderMode(test_shader);
        SetShaderMat4(test_shader, "uModel", mat4_identity());
        SetShaderMat4(test_shader, "uView", view);
        SetShaderMat4(test_shader, "uProjection", projection);
        SetShaderVec3(test_shader, "uLightDirection", normalize3(vec3(-0.5f, -1.0f, -0.35f)));
        SetShaderInt(test_shader, "uDiffuseMap", 0);
        for (mesh_index = 0; mesh_index < test_model.mesh_count; ++mesh_index)
        {
            ModelMesh mesh = GetModelMesh(test_model, mesh_index);
            BindTexture(mesh_textures[mesh_index], 0);
            RenderMesh(mesh.mesh);
            UnbindTexture(0);
        }
        EndShaderMode();
        EndDrawing();
    }

    ReleaseModel(test_model);
    ReleaseShader(test_shader);
    for (texture_index = 0; texture_index < ARRAY_COUNT(texture_paths); ++texture_index)
    {
        ReleaseTexture(mesh_textures[texture_index]);
    }

    CloseWindow();
    return 0;
}
