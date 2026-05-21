#include "core.h"
#include "core_internal.h"
#include <glad/glad.h>
#include "GLFW/glfw3.h"
#include "utils/log.h"

GLFWwindow *window;

static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    (void)window;
    glViewport(0, 0, width, height);
}


void InitWindow(int width, int height, const char* title)
{
	glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(width, height, title, NULL, NULL);

    if (window == NULL)
    {
        PANIC("window create failed");
        /* glfwTerminate(); */
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        PANIC("glad initialize failed");
    }

    glEnable(GL_DEPTH_TEST);

    {
        int framebuffer_width;
        int framebuffer_height;
        glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
        glViewport(0, 0, framebuffer_width, framebuffer_height);
    }
}

bool WindowShouldClose()
{
    if(window == NULL)
    {
        PANIC("window is null");
    }
    return glfwWindowShouldClose(window);
}

double GetTime(void)
{
    return glfwGetTime();
}

void GetFramebufferSize(int* width, int* height)
{
    if (window == NULL)
    {
        PANIC("window is null");
    }
    glfwGetFramebufferSize(window, width, height);
}

void ClearBackground(Color color)
{
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void CloseWindow()
{
    glfwTerminate();
}

void BeginDrawing()
{
    // nothing to do yet
}

void EndDrawing()
{
    glfwSwapBuffers(window);
}

GLFWwindow* core_get_window(void)
{
    return window;
}
