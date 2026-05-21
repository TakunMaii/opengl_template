#ifndef MESH_H
#define MESH_H

#include "math/linear_algebra.h"
#include "color.h"

typedef struct {
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    int indices_number;
} Mesh;

// a, b, c: screen space
Mesh GenerateTriangle2D(Vec2 a, Vec2 b, Vec2 c, Color color);

// center, size: screen space
Mesh GenerateRectangle2D(Vec2 center, Vec2 size, Color color);

void RenderMesh(Mesh mesh);

void ReleaseMesh(Mesh mesh);

#endif /* MESH_H */

