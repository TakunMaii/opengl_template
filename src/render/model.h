#ifndef MODEL_H
#define MODEL_H

#include "mesh.h"
#include <stddef.h>

typedef struct {
    Mesh *meshes;    
    size_t mesh_count;
} Model;

Model LoadModelFromObj(const char* obj_path);
void RenderModel(Model model);
void ReleaseModel(Model model);

#endif
