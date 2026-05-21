#ifndef SHADER_H
#define SHADER_H

#include "glad/glad.h"
#include "math/linear_algebra.h"

typedef struct {
    GLuint program;
} Shader;

Shader LoadShader(const char* vertex_shader_path, const char* fragment_shader_path);
void BeginShaderMode(Shader shader);
void EndShaderMode();
void SetShaderFloat(Shader shader, const char* name, float value);
void SetShaderInt(Shader shader, const char* name, int value);
void SetShaderVec2(Shader shader, const char* name, Vec2 value);
void SetShaderMat4(Shader shader, const char* name, Mat4 value);
void SetShaderVec3(Shader shader, const char* name, Vec3 value);
void SetShaderVec4(Shader shader, const char* name, Vec4 value);
void ReleaseShader(Shader shader);

#endif /* SHADER_H */
