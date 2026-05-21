#include <stdbool.h>
#include "core/core.h"
#include "math/linear_algebra.h"
#include "render/color.h"
#include "render/mesh.h"
#include "shader/shader.h"

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    InitWindow(800, 600, "OpenGl template");
    Mesh test_mesh = GenerateRectangle2D(vec2(0.f, 0.f), vec2(0.3f, 0.3f),colorFromHex(0x0000FFFF));
    Shader test_shader = LoadShader("assets/shaders/test.vs", "assets/shaders/test.fs");

    while (!WindowShouldClose())
    {
        ClearBackground(color(0.3f, 0.4f, 0.5f, 1.0f));

        BeginDrawing();
        BeginShaderMode(test_shader);
        RenderMesh(test_mesh);
        EndShaderMode();
        EndDrawing();
    }

    ReleaseMesh(test_mesh);
    ReleaseShader(test_shader);

    CloseWindow();
    return 0;
}
