#ifndef MODEL_H
#define MODEL_H

#include "mesh.h"
#include <stddef.h>

typedef struct {
    const char* name;
    Mesh mesh;
} ModelMesh;

typedef struct {
    ModelMesh* meshes;
    size_t mesh_count;
} Model;

Model LoadModelFromObj(const char* obj_path);
ModelMesh GetModelMesh(Model model, size_t index);
void RenderModel(Model model);
void ReleaseModel(Model model);

#endif
