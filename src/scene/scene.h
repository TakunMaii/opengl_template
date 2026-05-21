#ifndef SCENE_H
#define SCENE_H

#include "math/linear_algebra.h"
#include "render/mesh.h"
#include "render/texture.h"
#include <stdbool.h>
#include <stddef.h>

#define SCENE_MAX_SKIN_JOINTS 128

typedef enum {
    MATERIAL_ALPHA_OPAQUE,
    MATERIAL_ALPHA_MASK,
    MATERIAL_ALPHA_BLEND
} MaterialAlphaMode;

typedef struct {
    Texture* texture;
    int texcoord_set;
    Vec2 offset;
    Vec2 scale;
    float rotation;
    float strength;
    bool enabled;
} MaterialTextureSlot;

typedef struct {
    char* name;
    Vec4 base_color_factor;
    float metallic_factor;
    float roughness_factor;
    Vec3 emissive_factor;
    float alpha_cutoff;
    bool double_sided;
    bool unlit;
    MaterialAlphaMode alpha_mode;
    MaterialTextureSlot base_color_texture;
    MaterialTextureSlot metallic_roughness_texture;
    MaterialTextureSlot normal_texture;
    MaterialTextureSlot occlusion_texture;
    MaterialTextureSlot emissive_texture;
} Material;

typedef struct {
    char* name;
    Mesh mesh;
    Material* material;
    int skin_index;
    bool skinned;
    bool has_tangents;
} Primitive;

typedef struct {
    char* name;
    Primitive* primitives;
    size_t primitive_count;
} SceneMesh;

typedef struct {
    char* name;
    int* joints;
    size_t joint_count;
    int skeleton_root;
    Mat4* inverse_bind_matrices;
} Skin;

typedef struct {
    char* name;
    int mesh_index;
    int skin_index;
    int parent_index;
    int* child_indices;
    size_t child_count;
    Vec3 base_translation;
    Quat base_rotation;
    Vec3 base_scale;
    Mat4 base_matrix;
    bool uses_matrix;
    Vec3 local_translation;
    Quat local_rotation;
    Vec3 local_scale;
    Mat4 local_matrix;
    Mat4 world_matrix;
} SceneNode;

typedef enum {
    ANIMATION_PATH_TRANSLATION,
    ANIMATION_PATH_ROTATION,
    ANIMATION_PATH_SCALE,
    ANIMATION_PATH_WEIGHTS
} AnimationPath;

typedef enum {
    ANIMATION_INTERPOLATION_LINEAR,
    ANIMATION_INTERPOLATION_STEP,
    ANIMATION_INTERPOLATION_CUBIC
} AnimationInterpolation;

typedef struct {
    float* keyframes;
    float* values;
    size_t keyframe_count;
    int component_count;
    AnimationInterpolation interpolation;
} AnimationSampler;

typedef struct {
    int node_index;
    AnimationPath path;
    AnimationSampler sampler;
} AnimationChannel;

typedef struct {
    char* name;
    AnimationChannel* channels;
    size_t channel_count;
    float duration;
} SceneAnimation;

typedef struct {
    char* source_path;
    Texture* textures;
    size_t texture_count;
    Material* materials;
    size_t material_count;
    SceneMesh* meshes;
    size_t mesh_count;
    SceneNode* nodes;
    size_t node_count;
    int* scene_roots;
    size_t scene_root_count;
    Skin* skins;
    size_t skin_count;
    SceneAnimation* animations;
    size_t animation_count;
} Scene;

Scene LoadSceneFromGltf(const char* path);
void ResetScenePose(Scene* scene);
void UpdateSceneAnimation(Scene* scene, size_t animation_index, float time_seconds, bool loop);
void UpdateSceneWorldTransforms(Scene* scene);
void ReleaseScene(Scene scene);

#endif /* SCENE_H */
