#include "mesh.h"
#include "color.h"
#include "math/linear_algebra.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stddef.h>

Mesh CreateMesh(const Vertex* vertices, size_t vertex_count,
        const unsigned int* indices, size_t index_count)
{
    unsigned int VBO, VAO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(Vertex), vertices, GL_STATIC_DRAW);

    if (indices != NULL && index_count > 0)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(unsigned int), indices, GL_STATIC_DRAW);
    }

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), (void*)offsetof(Vertex, texcoord));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), (void*)offsetof(Vertex, texcoord1));
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), (void*)offsetof(Vertex, tangent));
    glEnableVertexAttribArray(4);

    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(5);

    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), (void*)offsetof(Vertex, joints));
    glEnableVertexAttribArray(6);

    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), (void*)offsetof(Vertex, weights));
    glEnableVertexAttribArray(7);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return (Mesh) {
        .VAO = VAO,
        .VBO = VBO,
        .EBO = EBO,
        .primitive_mode = GL_TRIANGLES,
        .vertex_count = vertex_count,
        .index_count = index_count,
        .has_indices = indices != NULL && index_count > 0,
    };
}

Mesh GenerateTriangle2D(Vec2 a, Vec2 b, Vec2 c, Color color)
{
    Vertex vertices[] = {
        {
            .position = {a.x, a.y, 0.0f},
            .normal = {0.0f, 0.0f, 1.0f},
            .texcoord = {0.0f, 0.0f},
            .texcoord1 = {0.0f, 0.0f},
            .tangent = {1.0f, 0.0f, 0.0f, 1.0f},
            .color = color,
            .joints = {0.0f, 0.0f, 0.0f, 0.0f},
            .weights = {1.0f, 0.0f, 0.0f, 0.0f},
        },
        {
            .position = {b.x, b.y, 0.0f},
            .normal = {0.0f, 0.0f, 1.0f},
            .texcoord = {1.0f, 0.0f},
            .texcoord1 = {1.0f, 0.0f},
            .tangent = {1.0f, 0.0f, 0.0f, 1.0f},
            .color = color,
            .joints = {0.0f, 0.0f, 0.0f, 0.0f},
            .weights = {1.0f, 0.0f, 0.0f, 0.0f},
        },
        {
            .position = {c.x, c.y, 0.0f},
            .normal = {0.0f, 0.0f, 1.0f},
            .texcoord = {0.5f, 1.0f},
            .texcoord1 = {0.5f, 1.0f},
            .tangent = {1.0f, 0.0f, 0.0f, 1.0f},
            .color = color,
            .joints = {0.0f, 0.0f, 0.0f, 0.0f},
            .weights = {1.0f, 0.0f, 0.0f, 0.0f},
        },
    };
    unsigned int indices[] = {0, 1, 2};

    return CreateMesh(vertices, sizeof(vertices) / sizeof(vertices[0]),
            indices, sizeof(indices) / sizeof(indices[0]));
}

Mesh GenerateRectangle2D(Vec2 center, Vec2 size, Color color)
{
    Vec2 left_top = (Vec2) {center.x - size.x * 0.5f, center.y - size.y * 0.5f};
    Vec2 right_top = (Vec2) {center.x + size.x * 0.5f, center.y - size.y * 0.5f};
    Vec2 left_bottom = (Vec2) {center.x - size.x * 0.5f, center.y + size.y * 0.5f};
    Vec2 right_bottom = (Vec2) {center.x + size.x * 0.5f, center.y + size.y * 0.5f};
    Vertex vertices[] = {
        {
            .position = {left_top.x, left_top.y, 0.0f},
            .normal = {0.0f, 0.0f, 1.0f},
            .texcoord = {0.0f, 1.0f},
            .texcoord1 = {0.0f, 1.0f},
            .tangent = {1.0f, 0.0f, 0.0f, 1.0f},
            .color = color,
            .joints = {0.0f, 0.0f, 0.0f, 0.0f},
            .weights = {1.0f, 0.0f, 0.0f, 0.0f},
        },
        {
            .position = {right_top.x, right_top.y, 0.0f},
            .normal = {0.0f, 0.0f, 1.0f},
            .texcoord = {1.0f, 1.0f},
            .texcoord1 = {1.0f, 1.0f},
            .tangent = {1.0f, 0.0f, 0.0f, 1.0f},
            .color = color,
            .joints = {0.0f, 0.0f, 0.0f, 0.0f},
            .weights = {1.0f, 0.0f, 0.0f, 0.0f},
        },
        {
            .position = {left_bottom.x, left_bottom.y, 0.0f},
            .normal = {0.0f, 0.0f, 1.0f},
            .texcoord = {0.0f, 0.0f},
            .texcoord1 = {0.0f, 0.0f},
            .tangent = {1.0f, 0.0f, 0.0f, 1.0f},
            .color = color,
            .joints = {0.0f, 0.0f, 0.0f, 0.0f},
            .weights = {1.0f, 0.0f, 0.0f, 0.0f},
        },
        {
            .position = {right_bottom.x, right_bottom.y, 0.0f},
            .normal = {0.0f, 0.0f, 1.0f},
            .texcoord = {1.0f, 0.0f},
            .texcoord1 = {1.0f, 0.0f},
            .tangent = {1.0f, 0.0f, 0.0f, 1.0f},
            .color = color,
            .joints = {0.0f, 0.0f, 0.0f, 0.0f},
            .weights = {1.0f, 0.0f, 0.0f, 0.0f},
        },
    };
    unsigned int indices[] = {2, 1, 0, 2, 3, 1};

    return CreateMesh(vertices, sizeof(vertices) / sizeof(vertices[0]),
            indices, sizeof(indices) / sizeof(indices[0]));
}

void RenderMesh(Mesh mesh)
{
    glBindVertexArray(mesh.VAO);
    if (mesh.has_indices)
    {
        glDrawElements(mesh.primitive_mode, (GLsizei)mesh.index_count, GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawArrays(mesh.primitive_mode, 0, (GLsizei)mesh.vertex_count);
    }
    glBindVertexArray(0);
}

void RenderMeshWireframe(Mesh mesh)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    RenderMesh(mesh);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void ReleaseMesh(Mesh mesh)
{
    glDeleteVertexArrays(1, &mesh.VAO);
    glDeleteBuffers(1, &mesh.VBO);
    if (mesh.EBO != 0)
    {
        glDeleteBuffers(1, &mesh.EBO);
    }
}
