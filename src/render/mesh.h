#ifndef MESH_H
#define MESH_H

#include "math/linear_algebra.h"
#include "color.h"
#include <stddef.h>

typedef struct {
    Vec3 position;
    Vec3 normal;
    Vec2 texcoord;
    Vec2 texcoord1;
    Vec4 tangent;
    Color color;
    Vec4 joints;
    Vec4 weights;
} Vertex;

typedef struct {
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    unsigned int primitive_mode;
    size_t vertex_count;
    size_t index_count;
    int has_indices;
} Mesh;

Mesh CreateMesh(const Vertex* vertices, size_t vertex_count,
        const unsigned int* indices, size_t index_count);

// a, b, c: screen space
Mesh GenerateTriangle2D(Vec2 a, Vec2 b, Vec2 c, Color color);

// center, size: screen space
Mesh GenerateRectangle2D(Vec2 center, Vec2 size, Color color);

void RenderMesh(Mesh mesh);

void RenderMeshWireframe(Mesh mesh);

void ReleaseMesh(Mesh mesh);

#endif /* MESH_H */
