#ifndef SHADER_H
#define SHADER_H

#include "glad/glad.h"

typedef struct {
    GLuint program;
} Shader;

Shader LoadShader(const char* vertex_shader_path, const char* fragment_shader_path);
void UseShader(Shader shader);
void EndUseShader();
void ReleaseShader(Shader shader);

#endif /* SHADER_H */

