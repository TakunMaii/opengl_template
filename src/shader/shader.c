#include "shader.h"
#include "GLFW/glfw3.h"
#include "glad/glad.h"
#include "utils/file.h"
#include "utils/log.h"
#include <stdbool.h>

static bool compile_shader(GLuint shader, const char *source)
{
    int success;
    char info_log[512];

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, sizeof(info_log), NULL, info_log);
        PANIC("shader compile failed");
        return false;
    }

    return true;
}

static bool link_program(GLuint program)
{
    int success;
    char info_log[512];

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, sizeof(info_log), NULL, info_log);
        PANIC("program link failed");
        return false;
    }

    return true;
}

Shader LoadShader(const char* vertex_shader_path, const char* fragment_shader_path)
{
    char* vertex_shader_source = read_entire_file(vertex_shader_path);
    char* fragment_shader_source = read_entire_file(fragment_shader_path);
    
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint shader_program = glCreateProgram();

    if (!compile_shader(vertex_shader, vertex_shader_source) ||
        !compile_shader(fragment_shader, fragment_shader_source))
    {
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        glfwTerminate();
        PANIC("shader compile failed");
    }

    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    if (!link_program(shader_program))
    {
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        glDeleteProgram(shader_program);
        glfwTerminate();
        PANIC("program link failed");
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return (Shader){
        .program = shader_program 
    };
}

void BeginShaderMode(Shader shader)
{
    glUseProgram(shader.program);
}

void EndShaderMode()
{
    glUseProgram(0);
}

void ReleaseShader(Shader shader)
{
    glDeleteProgram(shader.program);
}
