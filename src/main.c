#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdbool.h>
#include "math/linear_algebra.h"
#include "render/color.h"
#include "render/mesh.h"
#include "shader/shader.h"

static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    (void)window;
    glViewport(0, 0, width, height);
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

	glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window;

    window = glfwCreateWindow(800, 600, "OpenGL template", NULL, NULL);

    if (window == NULL)
    {
        printf("window create failed\n");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("glad initialize failed\n");
        return -1;
    }

    {
        int framebuffer_width;
        int framebuffer_height;
        glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
        glViewport(0, 0, framebuffer_width, framebuffer_height);
    }
    
    Mesh test_mesh = GenerateRectangle2D(vec2(0.f, 0.f), vec2(0.3f, 0.3f),colorFromHex(0x0000FFFF));
    Shader test_shader = LoadShader("assets/shaders/test.vs", "assets/shaders/test.fs");

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        UseShader(test_shader);
        RenderMesh(test_mesh);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ReleaseMesh(test_mesh);
    ReleaseShader(test_shader);

    glfwTerminate();
    return 0;
}
