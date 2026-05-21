#include "model.h"

#include "math/linear_algebra.h"
#include "mesh.h"
#include "utils/file.h"
#include "utils/log.h"
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    Vec3* data;
    size_t count;
    size_t capacity;
} Vec3Array;

typedef struct {
    Vec2* data;
    size_t count;
    size_t capacity;
} Vec2Array;

typedef struct {
    Vertex* data;
    size_t count;
    size_t capacity;
} VertexArray;

typedef struct {
    unsigned int* data;
    size_t count;
    size_t capacity;
} UIntArray;

typedef struct {
    Mesh* data;
    size_t count;
    size_t capacity;
} MeshArray;

typedef struct {
    int position_index;
    int texcoord_index;
    int normal_index;
} ObjIndexKey;

typedef struct {
    ObjIndexKey* data;
    size_t count;
    size_t capacity;
} ObjIndexKeyArray;

static void* checked_realloc(void* ptr, size_t size)
{
    void* result = realloc(ptr, size);
    if (result == NULL)
    {
        PANIC("unable to realloc memory");
    }
    return result;
}

static void ensure_vec3_capacity(Vec3Array* array, size_t min_capacity)
{
    size_t new_capacity;

    if (array->capacity >= min_capacity)
    {
        return;
    }

    new_capacity = array->capacity == 0 ? 16 : array->capacity;
    while (new_capacity < min_capacity)
    {
        new_capacity *= 2;
    }

    array->data = checked_realloc(array->data, new_capacity * sizeof(Vec3));
    array->capacity = new_capacity;
}

static void ensure_vec2_capacity(Vec2Array* array, size_t min_capacity)
{
    size_t new_capacity;

    if (array->capacity >= min_capacity)
    {
        return;
    }

    new_capacity = array->capacity == 0 ? 16 : array->capacity;
    while (new_capacity < min_capacity)
    {
        new_capacity *= 2;
    }

    array->data = checked_realloc(array->data, new_capacity * sizeof(Vec2));
    array->capacity = new_capacity;
}

static void ensure_vertex_capacity(VertexArray* array, size_t min_capacity)
{
    size_t new_capacity;

    if (array->capacity >= min_capacity)
    {
        return;
    }

    new_capacity = array->capacity == 0 ? 64 : array->capacity;
    while (new_capacity < min_capacity)
    {
        new_capacity *= 2;
    }

    array->data = checked_realloc(array->data, new_capacity * sizeof(Vertex));
    array->capacity = new_capacity;
}

static void ensure_uint_capacity(UIntArray* array, size_t min_capacity)
{
    size_t new_capacity;

    if (array->capacity >= min_capacity)
    {
        return;
    }

    new_capacity = array->capacity == 0 ? 128 : array->capacity;
    while (new_capacity < min_capacity)
    {
        new_capacity *= 2;
    }

    array->data = checked_realloc(array->data, new_capacity * sizeof(unsigned int));
    array->capacity = new_capacity;
}

static void ensure_mesh_capacity(MeshArray* array, size_t min_capacity)
{
    size_t new_capacity;

    if (array->capacity >= min_capacity)
    {
        return;
    }

    new_capacity = array->capacity == 0 ? 4 : array->capacity;
    while (new_capacity < min_capacity)
    {
        new_capacity *= 2;
    }

    array->data = checked_realloc(array->data, new_capacity * sizeof(Mesh));
    array->capacity = new_capacity;
}

static void ensure_key_capacity(ObjIndexKeyArray* array, size_t min_capacity)
{
    size_t new_capacity;

    if (array->capacity >= min_capacity)
    {
        return;
    }

    new_capacity = array->capacity == 0 ? 64 : array->capacity;
    while (new_capacity < min_capacity)
    {
        new_capacity *= 2;
    }

    array->data = checked_realloc(array->data, new_capacity * sizeof(ObjIndexKey));
    array->capacity = new_capacity;
}

static void push_vec3(Vec3Array* array, Vec3 value)
{
    ensure_vec3_capacity(array, array->count + 1);
    array->data[array->count++] = value;
}

static void push_vec2(Vec2Array* array, Vec2 value)
{
    ensure_vec2_capacity(array, array->count + 1);
    array->data[array->count++] = value;
}

static void push_vertex(VertexArray* array, Vertex value)
{
    ensure_vertex_capacity(array, array->count + 1);
    array->data[array->count++] = value;
}

static void push_uint(UIntArray* array, unsigned int value)
{
    ensure_uint_capacity(array, array->count + 1);
    array->data[array->count++] = value;
}

static void push_mesh(MeshArray* array, Mesh value)
{
    ensure_mesh_capacity(array, array->count + 1);
    array->data[array->count++] = value;
}

static void push_key(ObjIndexKeyArray* array, ObjIndexKey value)
{
    ensure_key_capacity(array, array->count + 1);
    array->data[array->count++] = value;
}

static void skip_spaces(char** cursor)
{
    while (**cursor == ' ' || **cursor == '\t' || **cursor == '\r')
    {
        (*cursor)++;
    }
}

static void skip_rest_of_line(char** cursor)
{
    while (**cursor != '\0' && **cursor != '\n')
    {
        (*cursor)++;
    }
    if (**cursor == '\n')
    {
        (*cursor)++;
    }
}

static bool starts_with_token(const char* cursor, const char* token)
{
    size_t token_length = strlen(token);
    char next_char = cursor[token_length];

    if (strncmp(cursor, token, token_length) != 0)
    {
        return false;
    }

    return next_char == ' ' || next_char == '\t' || next_char == '\r' || next_char == '\n' || next_char == '\0';
}

static int parse_int_token(char** cursor)
{
    long value;
    char* end;

    value = strtol(*cursor, &end, 10);
    if (end == *cursor)
    {
        PANIC("failed to parse obj integer");
    }

    *cursor = end;
    return (int)value;
}

static float parse_float_token(char** cursor)
{
    float value;
    char* end;

    value = strtof(*cursor, &end);
    if (end == *cursor)
    {
        PANIC("failed to parse obj float");
    }

    *cursor = end;
    return value;
}

static int resolve_obj_index(int index, size_t count)
{
    if (index > 0)
    {
        if ((size_t)index > count)
        {
            PANIC("obj index out of range");
        }
        return index - 1;
    }

    if (index < 0)
    {
        int resolved = (int)count + index;
        if (resolved < 0 || (size_t)resolved >= count)
        {
            PANIC("negative obj index out of range");
        }
        return resolved;
    }

    PANIC("obj index must not be zero");
    return -1;
}

static ObjIndexKey parse_face_vertex(char** cursor, const Vec3Array* positions,
        const Vec2Array* texcoords, const Vec3Array* normals)
{
    ObjIndexKey key;
    int raw_position_index;

    key.position_index = -1;
    key.texcoord_index = -1;
    key.normal_index = -1;

    raw_position_index = parse_int_token(cursor);
    key.position_index = resolve_obj_index(raw_position_index, positions->count);

    if (**cursor == '/')
    {
        (*cursor)++;

        if (**cursor != '/')
        {
            int raw_texcoord_index = parse_int_token(cursor);
            key.texcoord_index = resolve_obj_index(raw_texcoord_index, texcoords->count);
        }

        if (**cursor == '/')
        {
            (*cursor)++;
            if (**cursor != ' ' && **cursor != '\t' && **cursor != '\r' &&
                **cursor != '\n' && **cursor != '\0')
            {
                int raw_normal_index = parse_int_token(cursor);
                key.normal_index = resolve_obj_index(raw_normal_index, normals->count);
            }
        }
    }

    return key;
}

static bool same_key(ObjIndexKey a, ObjIndexKey b)
{
    return a.position_index == b.position_index &&
        a.texcoord_index == b.texcoord_index &&
        a.normal_index == b.normal_index;
}

static unsigned int get_or_create_vertex(const ObjIndexKey key, const Vec3Array* positions,
        const Vec2Array* texcoords, const Vec3Array* normals, VertexArray* vertices,
        ObjIndexKeyArray* keys)
{
    size_t i;
    Vertex vertex;

    for (i = 0; i < keys->count; ++i)
    {
        if (same_key(keys->data[i], key))
        {
            return (unsigned int)i;
        }
    }

    vertex.position = positions->data[key.position_index];
    vertex.normal = key.normal_index >= 0 ? normals->data[key.normal_index] : vec3(0.0f, 0.0f, 1.0f);
    vertex.texcoord = key.texcoord_index >= 0 ? texcoords->data[key.texcoord_index] : vec2(0.0f, 0.0f);
    vertex.color = color(1.0f, 1.0f, 1.0f, 1.0f);

    push_vertex(vertices, vertex);
    push_key(keys, key);

    return (unsigned int)(vertices->count - 1);
}

static void reset_mesh_builder(VertexArray* vertices, UIntArray* indices, ObjIndexKeyArray* keys)
{
    vertices->count = 0;
    indices->count = 0;
    keys->count = 0;
}

static void flush_mesh_builder(MeshArray* meshes, VertexArray* vertices, UIntArray* indices,
        ObjIndexKeyArray* keys)
{
    Mesh mesh;

    if (indices->count == 0)
    {
        reset_mesh_builder(vertices, indices, keys);
        return;
    }

    mesh = CreateMesh(vertices->data, vertices->count, indices->data, indices->count);
    push_mesh(meshes, mesh);
    reset_mesh_builder(vertices, indices, keys);
}

static void parse_vertex_position(char** cursor, Vec3Array* positions)
{
    float x;
    float y;
    float z;

    skip_spaces(cursor);
    x = parse_float_token(cursor);
    skip_spaces(cursor);
    y = parse_float_token(cursor);
    skip_spaces(cursor);
    z = parse_float_token(cursor);
    push_vec3(positions, vec3(x, y, z));
    skip_rest_of_line(cursor);
}

static void parse_vertex_texcoord(char** cursor, Vec2Array* texcoords)
{
    float u;
    float v;

    skip_spaces(cursor);
    u = parse_float_token(cursor);
    skip_spaces(cursor);
    v = parse_float_token(cursor);
    push_vec2(texcoords, vec2(u, v));
    skip_rest_of_line(cursor);
}

static void parse_vertex_normal(char** cursor, Vec3Array* normals)
{
    float x;
    float y;
    float z;

    skip_spaces(cursor);
    x = parse_float_token(cursor);
    skip_spaces(cursor);
    y = parse_float_token(cursor);
    skip_spaces(cursor);
    z = parse_float_token(cursor);
    push_vec3(normals, vec3(x, y, z));
    skip_rest_of_line(cursor);
}

static void parse_face(char** cursor, const Vec3Array* positions, const Vec2Array* texcoords,
        const Vec3Array* normals, VertexArray* vertices, UIntArray* indices, ObjIndexKeyArray* keys)
{
    unsigned int face_indices[64];
    size_t face_vertex_count = 0;
    size_t i;

    skip_spaces(cursor);
    while (**cursor != '\0' && **cursor != '\n')
    {
        ObjIndexKey key;

        if (face_vertex_count >= sizeof(face_indices) / sizeof(face_indices[0]))
        {
            PANIC("obj face has too many vertices");
        }

        key = parse_face_vertex(cursor, positions, texcoords, normals);
        face_indices[face_vertex_count++] = get_or_create_vertex(
            key, positions, texcoords, normals, vertices, keys);

        skip_spaces(cursor);
    }

    if (face_vertex_count < 3)
    {
        PANIC("obj face needs at least 3 vertices");
    }

    for (i = 1; i + 1 < face_vertex_count; ++i)
    {
        push_uint(indices, face_indices[0]);
        push_uint(indices, face_indices[i]);
        push_uint(indices, face_indices[i + 1]);
    }

    skip_rest_of_line(cursor);
}

Model LoadModelFromObj(const char *obj_path)
{
    char* obj_text = read_entire_file(obj_path);
    char* cursor = obj_text;
    Vec3Array positions = {0};
    Vec2Array texcoords = {0};
    Vec3Array normals = {0};
    VertexArray vertices = {0};
    UIntArray indices = {0};
    ObjIndexKeyArray keys = {0};
    MeshArray meshes = {0};
    Model model;

    while (*cursor != '\0')
    {
        skip_spaces(&cursor);

        if (*cursor == '\0')
        {
            break;
        }

        if (*cursor == '\n')
        {
            cursor++;
            continue;
        }

        if (*cursor == '#')
        {
            skip_rest_of_line(&cursor);
            continue;
        }

        if (starts_with_token(cursor, "v"))
        {
            cursor += 1;
            parse_vertex_position(&cursor, &positions);
            continue;
        }

        if (starts_with_token(cursor, "vt"))
        {
            cursor += 2;
            parse_vertex_texcoord(&cursor, &texcoords);
            continue;
        }

        if (starts_with_token(cursor, "vn"))
        {
            cursor += 2;
            parse_vertex_normal(&cursor, &normals);
            continue;
        }

        if (starts_with_token(cursor, "f"))
        {
            cursor += 1;
            parse_face(&cursor, &positions, &texcoords, &normals, &vertices, &indices, &keys);
            continue;
        }

        if (starts_with_token(cursor, "o") || starts_with_token(cursor, "g"))
        {
            flush_mesh_builder(&meshes, &vertices, &indices, &keys);
            skip_rest_of_line(&cursor);
            continue;
        }

        skip_rest_of_line(&cursor);
    }

    flush_mesh_builder(&meshes, &vertices, &indices, &keys);

    free(obj_text);
    free(positions.data);
    free(texcoords.data);
    free(normals.data);
    free(vertices.data);
    free(indices.data);
    free(keys.data);

    model.meshes = meshes.data;
    model.mesh_count = meshes.count;

    return model;
}

void RenderModel(Model model)
{
    size_t i;

    for (i = 0; i < model.mesh_count; ++i)
    {
        RenderMesh(model.meshes[i]);
    }
}

void ReleaseModel(Model model)
{
    size_t i;

    for (i = 0; i < model.mesh_count; i++)
    {
        ReleaseMesh(model.meshes[i]);
    }
    free(model.meshes);
}
