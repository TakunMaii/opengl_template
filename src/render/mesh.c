#include "mesh.h"
#include "color.h"
#include "math/linear_algebra.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <string.h>

Mesh GenerateTriangle2D(Vec2 a, Vec2 b, Vec2 c, Color color)
{
    float vertices[21];
    memset(vertices, 0, sizeof(vertices));
    memcpy(vertices + 0, vec2_data(&a), sizeof(Vec2));
    memcpy(vertices + 7, vec2_data(&b), sizeof(Vec2));
    memcpy(vertices + 14, vec2_data(&c), sizeof(Vec2));
    memcpy(vertices + 3, color_data(&color), sizeof(Color));
    memcpy(vertices + 10, color_data(&color), sizeof(Color));
    memcpy(vertices + 17, color_data(&color), sizeof(Color));

    unsigned int indices[] = {0, 1, 2};
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    // load VBO data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // load EBO data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
            7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return (Mesh) {
        .VAO = VAO,
        .VBO = VBO,
        .EBO = EBO,
        .indices_number = 3,
    };
}

Mesh GenerateRectangle2D(Vec2 center, Vec2 size, Color color)
{
    Vec2 left_top = (Vec2) {center.x - size.x * 0.5f, center.y - size.y * 0.5f};
    Vec2 right_top = (Vec2) {center.x + size.x * 0.5f, center.y - size.y * 0.5f};
    Vec2 left_bottom = (Vec2) {center.x - size.x * 0.5f, center.y + size.y * 0.5f};
    Vec2 right_bottom = (Vec2) {center.x + size.x * 0.5f, center.y + size.y * 0.5f};

    float vertices[28];
    memset(vertices, 0, sizeof(vertices));
    memcpy(vertices + 0, vec2_data(&left_top), sizeof(Vec2));
    memcpy(vertices + 7, vec2_data(&right_top), sizeof(Vec2));
    memcpy(vertices + 14, vec2_data(&left_bottom), sizeof(Vec2));
    memcpy(vertices + 21, vec2_data(&right_bottom), sizeof(Vec2));
    memcpy(vertices + 3, color_data(&color), sizeof(Color));
    memcpy(vertices + 10, color_data(&color), sizeof(Color));
    memcpy(vertices + 17, color_data(&color), sizeof(Color));
    memcpy(vertices + 24, color_data(&color), sizeof(Color));

    unsigned int indices[] = {2, 1, 0, 2, 3, 1};
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    // load VBO data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // load EBO data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
            7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return (Mesh) {
        .VAO = VAO,
        .VBO = VBO,
        .EBO = EBO,
        .indices_number = 6
    };
}

void RenderMesh(Mesh mesh)
{
    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_TRIANGLES, mesh.indices_number, GL_UNSIGNED_INT, 0);
}

void ReleaseMesh(Mesh mesh)
{
    glDeleteVertexArrays(1, &mesh.VAO);
    glDeleteBuffers(1, &mesh.VBO);
    glDeleteBuffers(1, &mesh.EBO);
}
