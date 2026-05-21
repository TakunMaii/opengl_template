#include <stdbool.h>
#include <math.h>
#include "core/core.h"
#include "core/input.h"
#include "math/linear_algebra.h"
#include "render/renderer.h"
#include "scene/scene.h"

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

int main(void)
{
    Scene scene;
    Renderer renderer;
    RendererSettings renderer_settings;
    Vec3 camera_position = vec3(0.0f, 1.0f, 3.0f);
    Vec3 world_up = vec3(0.0f, 1.0f, 0.0f);
    float yaw = -90.0f;
    float pitch = -5.0f;
    float move_speed = 2.6f;
    float mouse_sensitivity = 0.08f;
    double last_frame_time;
    float animation_time = 0.0f;

    InitWindow(1280, 720, "glTF OpenGL Wrapper");
    InitInput();

    scene = LoadSceneFromGltf("assets/Zombie_World/Characters/glTF/Characters_GermanShepherd.gltf");
    renderer_settings = DefaultRendererSettings();
    renderer = CreateRenderer(1280, 720, renderer_settings);
    SetCursorCaptured(true);
    last_frame_time = GetTime();

    while (!WindowShouldClose())
    {
        double current_frame_time = GetTime();
        float delta_time = (float)(current_frame_time - last_frame_time);
        int framebuffer_width;
        int framebuffer_height;
        Vec2 mouse_delta;
        Vec3 camera_front;
        Vec3 camera_right;
        Vec3 camera_up;
        Vec3 move_forward;
        Mat4 projection;
        Mat4 view;
        Camera3D camera;

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

        if (IsKeyPressed(KEY_1))
        {
            renderer.settings.render_mode = RENDER_MODE_PBR;
        }
        if (IsKeyPressed(KEY_2))
        {
            renderer.settings.render_mode = RENDER_MODE_TOON;
        }
        if (IsKeyPressed(KEY_3))
        {
            renderer.settings.enable_outline = !renderer.settings.enable_outline;
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
        if (framebuffer_width <= 0) framebuffer_width = 1;
        if (framebuffer_height <= 0) framebuffer_height = 1;
        ResizeRenderer(&renderer, framebuffer_width, framebuffer_height);

        projection = mat4_perspective(
            radiansf(45.0f),
            (float)framebuffer_width / (float)framebuffer_height,
            0.01f,
            100.0f
        );
        view = mat4_look_at(camera_position, add3(camera_position, camera_front), camera_up);
        camera.view = view;
        camera.projection = projection;
        camera.position = camera_position;

        BeginDrawing();
        animation_time += delta_time;
        if (scene.animation_count > 0)
        {
            UpdateSceneAnimation(&scene, 0, animation_time, true);
        }
        else
        {
            UpdateSceneWorldTransforms(&scene);
        }
        RenderScene(&renderer, &scene, camera);
        EndDrawing();
    }

    ReleaseRenderer(renderer);
    ReleaseScene(scene);
    CloseWindow();
    return 0;
}
