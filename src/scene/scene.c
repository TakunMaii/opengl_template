#define CGLTF_IMPLEMENTATION
#include "scene.h"

#include "cgltf.h"
#include "render/color.h"
#include "utils/file.h"
#include "utils/log.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    cgltf_data* gltf;
    Scene scene;
    int* node_map;
} GltfBuildContext;

static void* checked_calloc(size_t count, size_t size)
{
    void* memory = calloc(count, size);
    if (memory == NULL)
    {
        PANIC("unable to allocate memory");
    }
    return memory;
}

static char* duplicate_string(const char* text)
{
    size_t length;
    char* result;

    if (text == NULL)
    {
        return NULL;
    }

    length = strlen(text) + 1;
    result = checked_calloc(length, sizeof(char));
    memcpy(result, text, length);
    return result;
}

static int pointer_index(const void* item, const void* base, size_t stride, size_t count)
{
    size_t i;
    const unsigned char* target = (const unsigned char*)item;
    const unsigned char* begin = (const unsigned char*)base;

    if (item == NULL || base == NULL)
    {
        return -1;
    }

    for (i = 0; i < count; ++i)
    {
        if (begin + i * stride == target)
        {
            return (int)i;
        }
    }

    return -1;
}

static Vec3 vec3_from_array(const float* values)
{
    return vec3(values[0], values[1], values[2]);
}

static Vec4 vec4_from_array(const float* values)
{
    return vec4(values[0], values[1], values[2], values[3]);
}

static Quat quat_from_array(const float* values)
{
    return normalize_quat(quat(values[0], values[1], values[2], values[3]));
}

static Mat4 local_matrix_from_node(const cgltf_node* node)
{
    if (node->has_matrix)
    {
        return mat4_from_column_major(node->matrix);
    }

    return mat4_from_trs(
        node->has_translation ? vec3_from_array(node->translation) : vec3(0.0f, 0.0f, 0.0f),
        node->has_rotation ? quat_from_array(node->rotation) : quat(0.0f, 0.0f, 0.0f, 1.0f),
        node->has_scale ? vec3_from_array(node->scale) : vec3(1.0f, 1.0f, 1.0f)
    );
}

static void decompose_node(const cgltf_node* source, SceneNode* destination)
{
    destination->uses_matrix = source->has_matrix;
    destination->base_translation = source->has_translation ? vec3_from_array(source->translation) : vec3(0.0f, 0.0f, 0.0f);
    destination->base_rotation = source->has_rotation ? quat_from_array(source->rotation) : quat(0.0f, 0.0f, 0.0f, 1.0f);
    destination->base_scale = source->has_scale ? vec3_from_array(source->scale) : vec3(1.0f, 1.0f, 1.0f);
    destination->base_matrix = local_matrix_from_node(source);
    destination->local_translation = destination->base_translation;
    destination->local_rotation = destination->base_rotation;
    destination->local_scale = destination->base_scale;
    destination->local_matrix = destination->base_matrix;
    destination->world_matrix = destination->base_matrix;
}

static void apply_node_local_transform(SceneNode* node)
{
    if (node->uses_matrix)
    {
        node->local_matrix = node->base_matrix;
    }
    else
    {
        node->local_matrix = mat4_from_trs(node->local_translation, node->local_rotation, node->local_scale);
    }
}

static GLenum primitive_mode_from_gltf(cgltf_primitive_type type)
{
    switch (type)
    {
    case cgltf_primitive_type_points:
        return GL_POINTS;
    case cgltf_primitive_type_lines:
        return GL_LINES;
    case cgltf_primitive_type_line_loop:
        return GL_LINE_LOOP;
    case cgltf_primitive_type_line_strip:
        return GL_LINE_STRIP;
    case cgltf_primitive_type_triangles:
        return GL_TRIANGLES;
    case cgltf_primitive_type_triangle_strip:
        return GL_TRIANGLE_STRIP;
    case cgltf_primitive_type_triangle_fan:
        return GL_TRIANGLE_FAN;
    default:
        return GL_TRIANGLES;
    }
}

static void material_slot_defaults(MaterialTextureSlot* slot)
{
    memset(slot, 0, sizeof(*slot));
    slot->scale = vec2(1.0f, 1.0f);
    slot->strength = 1.0f;
}

static void apply_texture_transform(MaterialTextureSlot* slot, const cgltf_texture_view* view)
{
    slot->enabled = view->texture != NULL;
    slot->texcoord_set = view->texcoord;
    slot->strength = view->scale;
    slot->offset = vec2(0.0f, 0.0f);
    slot->scale = vec2(1.0f, 1.0f);
    slot->rotation = 0.0f;

    if (view->has_transform)
    {
        slot->offset = vec2(view->transform.offset[0], view->transform.offset[1]);
        slot->scale = vec2(view->transform.scale[0], view->transform.scale[1]);
        slot->rotation = view->transform.rotation;
        if (view->transform.has_texcoord)
        {
            slot->texcoord_set = view->transform.texcoord;
        }
    }
}

static GLint wrap_mode_from_gltf(cgltf_wrap_mode mode)
{
    switch (mode)
    {
    case cgltf_wrap_mode_clamp_to_edge:
        return GL_CLAMP_TO_EDGE;
    case cgltf_wrap_mode_mirrored_repeat:
        return GL_MIRRORED_REPEAT;
    case cgltf_wrap_mode_repeat:
    default:
        return GL_REPEAT;
    }
}

static GLint filter_mode_from_gltf(cgltf_filter_type mode, GLint fallback)
{
    if (mode == cgltf_filter_type_undefined)
    {
        return fallback;
    }

    return (GLint)mode;
}

static Texture load_texture_from_image(const char* scene_path, const cgltf_texture* texture_source)
{
    TextureLoadOptions options = DefaultTextureLoadOptions();
    const cgltf_image* image;
    const cgltf_sampler* sampler;

    if (texture_source == NULL)
    {
        PANIC("texture source is null");
    }

    image = texture_source->image;
    sampler = texture_source->sampler;

    if (sampler != NULL)
    {
        options.wrap_s = wrap_mode_from_gltf(sampler->wrap_s);
        options.wrap_t = wrap_mode_from_gltf(sampler->wrap_t);
        options.min_filter = filter_mode_from_gltf(sampler->min_filter, GL_LINEAR_MIPMAP_LINEAR);
        options.mag_filter = filter_mode_from_gltf(sampler->mag_filter, GL_LINEAR);
    }
    options.flip_y = 0;

    if (image == NULL)
    {
        return CreateSolidTexture(color(1.0f, 1.0f, 1.0f, 1.0f));
    }

    if (image->buffer_view != NULL)
    {
        const unsigned char* bytes = (const unsigned char*)cgltf_buffer_view_data(image->buffer_view);
        int length = (int)image->buffer_view->size;
        return LoadTextureFromMemory(bytes, length, options);
    }

    if (image->uri != NULL)
    {
        size_t directory_length = 0;
        size_t i;
        char* joined_path;

        for (i = 0; scene_path[i] != '\0'; ++i)
        {
            if (scene_path[i] == '/' || scene_path[i] == '\\')
            {
                directory_length = i + 1;
            }
        }

        joined_path = checked_calloc(directory_length + strlen(image->uri) + 1, sizeof(char));
        memcpy(joined_path, scene_path, directory_length);
        strcpy(joined_path + directory_length, image->uri);

        {
            Texture texture = LoadTextureEx(joined_path, options);
            free(joined_path);
            return texture;
        }
    }

    return CreateSolidTexture(color(1.0f, 1.0f, 1.0f, 1.0f));
}

static void load_textures(GltfBuildContext* context, const char* path)
{
    size_t i;
    context->scene.texture_count = context->gltf->textures_count;
    if (context->scene.texture_count == 0)
    {
        context->scene.textures = NULL;
        return;
    }

    context->scene.textures = checked_calloc(context->scene.texture_count, sizeof(Texture));
    for (i = 0; i < context->scene.texture_count; ++i)
    {
        context->scene.textures[i] = load_texture_from_image(path, &context->gltf->textures[i]);
    }
}

static Texture* texture_pointer_from_view(GltfBuildContext* context, const cgltf_texture_view* view)
{
    int index;

    if (view == NULL || view->texture == NULL)
    {
        return NULL;
    }

    index = pointer_index(view->texture, context->gltf->textures, sizeof(cgltf_texture), context->gltf->textures_count);
    if (index < 0)
    {
        return NULL;
    }
    return &context->scene.textures[index];
}

static void load_materials(GltfBuildContext* context)
{
    size_t i;

    context->scene.material_count = context->gltf->materials_count;
    if (context->scene.material_count == 0)
    {
        context->scene.materials = checked_calloc(1, sizeof(Material));
        context->scene.material_count = 1;
        context->scene.materials[0].base_color_factor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
        context->scene.materials[0].metallic_factor = 1.0f;
        context->scene.materials[0].roughness_factor = 1.0f;
        return;
    }

    context->scene.materials = checked_calloc(context->scene.material_count, sizeof(Material));
    for (i = 0; i < context->scene.material_count; ++i)
    {
        const cgltf_material* source = &context->gltf->materials[i];
        Material* destination = &context->scene.materials[i];

        destination->name = duplicate_string(source->name);
        destination->base_color_factor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
        destination->metallic_factor = 1.0f;
        destination->roughness_factor = 1.0f;
        destination->emissive_factor = vec3(0.0f, 0.0f, 0.0f);
        destination->alpha_cutoff = source->alpha_cutoff > 0.0f ? source->alpha_cutoff : 0.5f;
        destination->double_sided = source->double_sided != 0;
        destination->unlit = source->unlit != 0;
        destination->alpha_mode = MATERIAL_ALPHA_OPAQUE;
        material_slot_defaults(&destination->base_color_texture);
        material_slot_defaults(&destination->metallic_roughness_texture);
        material_slot_defaults(&destination->normal_texture);
        material_slot_defaults(&destination->occlusion_texture);
        material_slot_defaults(&destination->emissive_texture);

        if (source->alpha_mode == cgltf_alpha_mode_mask)
        {
            destination->alpha_mode = MATERIAL_ALPHA_MASK;
        }
        else if (source->alpha_mode == cgltf_alpha_mode_blend)
        {
            destination->alpha_mode = MATERIAL_ALPHA_BLEND;
        }

        if (source->has_pbr_metallic_roughness)
        {
            destination->base_color_factor = vec4_from_array(source->pbr_metallic_roughness.base_color_factor);
            destination->metallic_factor = source->pbr_metallic_roughness.metallic_factor;
            destination->roughness_factor = source->pbr_metallic_roughness.roughness_factor;

            destination->base_color_texture.texture = texture_pointer_from_view(context, &source->pbr_metallic_roughness.base_color_texture);
            apply_texture_transform(&destination->base_color_texture, &source->pbr_metallic_roughness.base_color_texture);

            destination->metallic_roughness_texture.texture = texture_pointer_from_view(context, &source->pbr_metallic_roughness.metallic_roughness_texture);
            apply_texture_transform(&destination->metallic_roughness_texture, &source->pbr_metallic_roughness.metallic_roughness_texture);
        }

        destination->emissive_factor = vec3_from_array(source->emissive_factor);
        destination->normal_texture.texture = texture_pointer_from_view(context, &source->normal_texture);
        apply_texture_transform(&destination->normal_texture, &source->normal_texture);
        destination->occlusion_texture.texture = texture_pointer_from_view(context, &source->occlusion_texture);
        apply_texture_transform(&destination->occlusion_texture, &source->occlusion_texture);
        destination->emissive_texture.texture = texture_pointer_from_view(context, &source->emissive_texture);
        apply_texture_transform(&destination->emissive_texture, &source->emissive_texture);
    }
}

static bool read_vec2_accessor(const cgltf_accessor* accessor, size_t index, Vec2* out)
{
    float values[4] = {0};
    if (accessor == NULL || !cgltf_accessor_read_float(accessor, index, values, 4))
    {
        return false;
    }
    *out = vec2(values[0], values[1]);
    return true;
}

static bool read_vec3_accessor(const cgltf_accessor* accessor, size_t index, Vec3* out)
{
    float values[4] = {0};
    if (accessor == NULL || !cgltf_accessor_read_float(accessor, index, values, 4))
    {
        return false;
    }
    *out = vec3(values[0], values[1], values[2]);
    return true;
}

static bool read_vec4_accessor(const cgltf_accessor* accessor, size_t index, Vec4* out)
{
    float values[4] = {0};
    if (accessor == NULL || !cgltf_accessor_read_float(accessor, index, values, 4))
    {
        return false;
    }
    *out = vec4(values[0], values[1], values[2], values[3]);
    return true;
}

static bool read_color_accessor(const cgltf_accessor* accessor, size_t index, Color* out)
{
    float values[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    if (accessor == NULL || !cgltf_accessor_read_float(accessor, index, values, 4))
    {
        return false;
    }
    if (cgltf_num_components(accessor->type) == 3)
    {
        values[3] = 1.0f;
    }
    *out = color(values[0], values[1], values[2], values[3]);
    return true;
}

static bool read_uint4_accessor(const cgltf_accessor* accessor, size_t index, Vec4* out)
{
    cgltf_uint values[4] = {0};
    if (accessor == NULL || !cgltf_accessor_read_uint(accessor, index, values, 4))
    {
        return false;
    }
    *out = vec4((float)values[0], (float)values[1], (float)values[2], (float)values[3]);
    return true;
}

static void build_primitive_vertices(const cgltf_primitive* primitive, Vertex** vertices_out, size_t* vertex_count_out, bool* has_tangents_out)
{
    const cgltf_accessor* positions = cgltf_find_accessor(primitive, cgltf_attribute_type_position, 0);
    const cgltf_accessor* normals = cgltf_find_accessor(primitive, cgltf_attribute_type_normal, 0);
    const cgltf_accessor* tangents = cgltf_find_accessor(primitive, cgltf_attribute_type_tangent, 0);
    const cgltf_accessor* texcoord0 = cgltf_find_accessor(primitive, cgltf_attribute_type_texcoord, 0);
    const cgltf_accessor* texcoord1 = cgltf_find_accessor(primitive, cgltf_attribute_type_texcoord, 1);
    const cgltf_accessor* colors = cgltf_find_accessor(primitive, cgltf_attribute_type_color, 0);
    const cgltf_accessor* joints = cgltf_find_accessor(primitive, cgltf_attribute_type_joints, 0);
    const cgltf_accessor* weights = cgltf_find_accessor(primitive, cgltf_attribute_type_weights, 0);
    size_t i;
    Vertex* vertices;
    size_t vertex_count;

    if (positions == NULL)
    {
        PANIC("primitive missing positions");
    }

    vertex_count = positions->count;
    vertices = checked_calloc(vertex_count, sizeof(Vertex));
    for (i = 0; i < vertex_count; ++i)
    {
        Vertex* vertex = &vertices[i];
        vertex->normal = vec3(0.0f, 0.0f, 1.0f);
        vertex->tangent = vec4(1.0f, 0.0f, 0.0f, 1.0f);
        vertex->color = color(1.0f, 1.0f, 1.0f, 1.0f);
        vertex->weights = vec4(1.0f, 0.0f, 0.0f, 0.0f);

        if (!read_vec3_accessor(positions, i, &vertex->position))
        {
            PANIC("unable to read position");
        }
        read_vec3_accessor(normals, i, &vertex->normal);
        read_vec4_accessor(tangents, i, &vertex->tangent);
        read_vec2_accessor(texcoord0, i, &vertex->texcoord);
        read_vec2_accessor(texcoord1, i, &vertex->texcoord1);
        read_color_accessor(colors, i, &vertex->color);
        read_uint4_accessor(joints, i, &vertex->joints);
        read_vec4_accessor(weights, i, &vertex->weights);
    }

    *vertices_out = vertices;
    *vertex_count_out = vertex_count;
    *has_tangents_out = tangents != NULL;
}

static unsigned int* build_primitive_indices(const cgltf_primitive* primitive, size_t vertex_count, size_t* index_count_out)
{
    size_t index_count;
    size_t i;
    unsigned int* indices;

    if (primitive->indices == NULL)
    {
        *index_count_out = 0;
        return NULL;
    }

    index_count = primitive->indices->count;
    indices = checked_calloc(index_count, sizeof(unsigned int));
    for (i = 0; i < index_count; ++i)
    {
        size_t index = cgltf_accessor_read_index(primitive->indices, i);
        if (index >= vertex_count)
        {
            free(indices);
            PANIC("index out of range");
        }
        indices[i] = (unsigned int)index;
    }

    *index_count_out = index_count;
    return indices;
}

static Material* material_pointer_for_primitive(GltfBuildContext* context, const cgltf_primitive* primitive)
{
    int index;
    if (primitive->material == NULL)
    {
        return &context->scene.materials[0];
    }

    index = pointer_index(primitive->material, context->gltf->materials, sizeof(cgltf_material), context->gltf->materials_count);
    if (index < 0)
    {
        return &context->scene.materials[0];
    }
    return &context->scene.materials[index];
}

static void load_meshes(GltfBuildContext* context)
{
    size_t i;

    context->scene.mesh_count = context->gltf->meshes_count;
    context->scene.meshes = context->scene.mesh_count > 0
        ? checked_calloc(context->scene.mesh_count, sizeof(SceneMesh))
        : NULL;

    for (i = 0; i < context->scene.mesh_count; ++i)
    {
        const cgltf_mesh* source_mesh = &context->gltf->meshes[i];
        SceneMesh* destination_mesh = &context->scene.meshes[i];
        size_t primitive_index;

        destination_mesh->name = duplicate_string(source_mesh->name);
        destination_mesh->primitive_count = source_mesh->primitives_count;
        destination_mesh->primitives = destination_mesh->primitive_count > 0
            ? checked_calloc(destination_mesh->primitive_count, sizeof(Primitive))
            : NULL;

        for (primitive_index = 0; primitive_index < destination_mesh->primitive_count; ++primitive_index)
        {
            const cgltf_primitive* source_primitive = &source_mesh->primitives[primitive_index];
            Primitive* destination_primitive = &destination_mesh->primitives[primitive_index];
            Vertex* vertices;
            unsigned int* indices;
            size_t vertex_count;
            size_t index_count;

            build_primitive_vertices(source_primitive, &vertices, &vertex_count, &destination_primitive->has_tangents);
            indices = build_primitive_indices(source_primitive, vertex_count, &index_count);

            destination_primitive->name = duplicate_string(source_mesh->name);
            destination_primitive->material = material_pointer_for_primitive(context, source_primitive);
            destination_primitive->mesh = CreateMesh(vertices, vertex_count, indices, index_count);
            destination_primitive->mesh.primitive_mode = primitive_mode_from_gltf(source_primitive->type);
            destination_primitive->skin_index = -1;
            destination_primitive->skinned = false;

            free(vertices);
            free(indices);
        }
    }
}

static void load_skins(GltfBuildContext* context)
{
    size_t i;
    context->scene.skin_count = context->gltf->skins_count;
    context->scene.skins = context->scene.skin_count > 0
        ? checked_calloc(context->scene.skin_count, sizeof(Skin))
        : NULL;

    for (i = 0; i < context->scene.skin_count; ++i)
    {
        const cgltf_skin* source = &context->gltf->skins[i];
        Skin* destination = &context->scene.skins[i];
        size_t joint_index;

        destination->name = duplicate_string(source->name);
        destination->joint_count = source->joints_count;
        destination->skeleton_root = pointer_index(source->skeleton, context->gltf->nodes, sizeof(cgltf_node), context->gltf->nodes_count);
        destination->joints = destination->joint_count > 0 ? checked_calloc(destination->joint_count, sizeof(int)) : NULL;
        destination->inverse_bind_matrices = destination->joint_count > 0 ? checked_calloc(destination->joint_count, sizeof(Mat4)) : NULL;

        for (joint_index = 0; joint_index < destination->joint_count; ++joint_index)
        {
            float matrix_values[16];
            destination->joints[joint_index] = pointer_index(
                source->joints[joint_index], context->gltf->nodes, sizeof(cgltf_node), context->gltf->nodes_count);

            if (source->inverse_bind_matrices != NULL &&
                cgltf_accessor_read_float(source->inverse_bind_matrices, joint_index, matrix_values, 16))
            {
                destination->inverse_bind_matrices[joint_index] = mat4_from_column_major(matrix_values);
            }
            else
            {
                destination->inverse_bind_matrices[joint_index] = mat4_identity();
            }
        }
    }
}

static void load_nodes(GltfBuildContext* context)
{
    size_t i;

    context->scene.node_count = context->gltf->nodes_count;
    context->scene.nodes = context->scene.node_count > 0
        ? checked_calloc(context->scene.node_count, sizeof(SceneNode))
        : NULL;
    context->node_map = context->scene.node_count > 0
        ? checked_calloc(context->scene.node_count, sizeof(int))
        : NULL;

    for (i = 0; i < context->scene.node_count; ++i)
    {
        const cgltf_node* source = &context->gltf->nodes[i];
        SceneNode* destination = &context->scene.nodes[i];
        size_t child_index;

        context->node_map[i] = (int)i;
        destination->name = duplicate_string(source->name);
        destination->mesh_index = pointer_index(source->mesh, context->gltf->meshes, sizeof(cgltf_mesh), context->gltf->meshes_count);
        destination->skin_index = pointer_index(source->skin, context->gltf->skins, sizeof(cgltf_skin), context->gltf->skins_count);
        destination->parent_index = pointer_index(source->parent, context->gltf->nodes, sizeof(cgltf_node), context->gltf->nodes_count);
        destination->child_count = source->children_count;
        destination->child_indices = destination->child_count > 0 ? checked_calloc(destination->child_count, sizeof(int)) : NULL;

        for (child_index = 0; child_index < destination->child_count; ++child_index)
        {
            destination->child_indices[child_index] = pointer_index(
                source->children[child_index], context->gltf->nodes, sizeof(cgltf_node), context->gltf->nodes_count);
        }

        decompose_node(source, destination);

        if (destination->mesh_index >= 0 && destination->skin_index >= 0)
        {
            SceneMesh* mesh = &context->scene.meshes[destination->mesh_index];
            size_t primitive_index;
            for (primitive_index = 0; primitive_index < mesh->primitive_count; ++primitive_index)
            {
                mesh->primitives[primitive_index].skin_index = destination->skin_index;
                mesh->primitives[primitive_index].skinned = true;
            }
        }
    }

    if (context->gltf->scene != NULL)
    {
        context->scene.scene_root_count = context->gltf->scene->nodes_count;
        context->scene.scene_roots = context->scene.scene_root_count > 0
            ? checked_calloc(context->scene.scene_root_count, sizeof(int))
            : NULL;
        for (i = 0; i < context->scene.scene_root_count; ++i)
        {
            context->scene.scene_roots[i] = pointer_index(
                context->gltf->scene->nodes[i], context->gltf->nodes, sizeof(cgltf_node), context->gltf->nodes_count);
        }
    }
}

static AnimationInterpolation interpolation_from_gltf(cgltf_interpolation_type type)
{
    switch (type)
    {
    case cgltf_interpolation_type_step:
        return ANIMATION_INTERPOLATION_STEP;
    case cgltf_interpolation_type_cubic_spline:
        return ANIMATION_INTERPOLATION_CUBIC;
    case cgltf_interpolation_type_linear:
    default:
        return ANIMATION_INTERPOLATION_LINEAR;
    }
}

static AnimationPath animation_path_from_gltf(cgltf_animation_path_type type)
{
    switch (type)
    {
    case cgltf_animation_path_type_translation:
        return ANIMATION_PATH_TRANSLATION;
    case cgltf_animation_path_type_rotation:
        return ANIMATION_PATH_ROTATION;
    case cgltf_animation_path_type_scale:
        return ANIMATION_PATH_SCALE;
    case cgltf_animation_path_type_weights:
    default:
        return ANIMATION_PATH_WEIGHTS;
    }
}

static void load_animations(GltfBuildContext* context)
{
    size_t i;

    context->scene.animation_count = context->gltf->animations_count;
    context->scene.animations = context->scene.animation_count > 0
        ? checked_calloc(context->scene.animation_count, sizeof(SceneAnimation))
        : NULL;

    for (i = 0; i < context->scene.animation_count; ++i)
    {
        const cgltf_animation* source = &context->gltf->animations[i];
        SceneAnimation* destination = &context->scene.animations[i];
        size_t channel_index;

        destination->name = duplicate_string(source->name);
        destination->channel_count = source->channels_count;
        destination->channels = destination->channel_count > 0
            ? checked_calloc(destination->channel_count, sizeof(AnimationChannel))
            : NULL;

        for (channel_index = 0; channel_index < destination->channel_count; ++channel_index)
        {
            const cgltf_animation_channel* source_channel = &source->channels[channel_index];
            const cgltf_animation_sampler* source_sampler = source_channel->sampler;
            AnimationChannel* destination_channel = &destination->channels[channel_index];
            AnimationSampler* destination_sampler = &destination_channel->sampler;
            size_t keyframe_index;

            destination_channel->node_index = pointer_index(
                source_channel->target_node, context->gltf->nodes, sizeof(cgltf_node), context->gltf->nodes_count);
            destination_channel->path = animation_path_from_gltf(source_channel->target_path);
            destination_sampler->keyframe_count = source_sampler->input != NULL ? source_sampler->input->count : 0;
            destination_sampler->interpolation = interpolation_from_gltf(source_sampler->interpolation);
            destination_sampler->component_count = (int)cgltf_num_components(source_sampler->output->type);
            if (destination_sampler->interpolation == ANIMATION_INTERPOLATION_CUBIC)
            {
                destination_sampler->component_count *= 3;
            }
            destination_sampler->keyframes = destination_sampler->keyframe_count > 0
                ? checked_calloc(destination_sampler->keyframe_count, sizeof(float))
                : NULL;
            destination_sampler->values = destination_sampler->keyframe_count > 0
                ? checked_calloc(destination_sampler->keyframe_count * (size_t)destination_sampler->component_count, sizeof(float))
                : NULL;

            for (keyframe_index = 0; keyframe_index < destination_sampler->keyframe_count; ++keyframe_index)
            {
                float frame_time = 0.0f;
                cgltf_accessor_read_float(source_sampler->input, keyframe_index, &frame_time, 1);
                destination_sampler->keyframes[keyframe_index] = frame_time;
                if (frame_time > destination->duration)
                {
                    destination->duration = frame_time;
                }

                if (!cgltf_accessor_read_float(
                        source_sampler->output,
                        keyframe_index,
                        &destination_sampler->values[keyframe_index * (size_t)destination_sampler->component_count],
                        (cgltf_size)destination_sampler->component_count))
                {
                    PANIC("unable to read animation values");
                }
            }
        }
    }
}

Scene LoadSceneFromGltf(const char* path)
{
    cgltf_options options = {0};
    cgltf_data* gltf = NULL;
    GltfBuildContext context;

    memset(&context, 0, sizeof(context));

    if (cgltf_parse_file(&options, path, &gltf) != cgltf_result_success)
    {
        PANIC("failed to parse gltf file");
    }
    if (cgltf_load_buffers(&options, gltf, path) != cgltf_result_success)
    {
        cgltf_free(gltf);
        PANIC("failed to load gltf buffers");
    }
    if (cgltf_validate(gltf) != cgltf_result_success)
    {
        cgltf_free(gltf);
        PANIC("failed to validate gltf");
    }

    context.gltf = gltf;
    context.scene.source_path = duplicate_string(path);

    load_textures(&context, path);
    load_materials(&context);
    load_meshes(&context);
    load_skins(&context);
    load_nodes(&context);
    load_animations(&context);
    UpdateSceneWorldTransforms(&context.scene);

    free(context.node_map);
    cgltf_free(gltf);
    return context.scene;
}

void ResetScenePose(Scene* scene)
{
    size_t i;
    if (scene == NULL)
    {
        return;
    }

    for (i = 0; i < scene->node_count; ++i)
    {
        scene->nodes[i].local_translation = scene->nodes[i].base_translation;
        scene->nodes[i].local_rotation = scene->nodes[i].base_rotation;
        scene->nodes[i].local_scale = scene->nodes[i].base_scale;
        scene->nodes[i].local_matrix = scene->nodes[i].base_matrix;
    }
}

static size_t find_keyframe_index(const AnimationSampler* sampler, float time_seconds)
{
    size_t low = 0;
    size_t high = sampler->keyframe_count - 1;

    while (low + 1 < high)
    {
        size_t mid = (low + high) / 2;
        if (sampler->keyframes[mid] <= time_seconds)
        {
            low = mid;
        }
        else
        {
            high = mid;
        }
    }

    return low;
}

static void sample_animation_vec(const AnimationSampler* sampler, float time_seconds, float* out_values)
{
    size_t base_index;
    size_t next_index;
    size_t component_count = (size_t)sampler->component_count;
    const float* a;
    const float* b;
    float frame_a;
    float frame_b;
    float t;
    size_t i;

    if (sampler->keyframe_count == 0)
    {
        memset(out_values, 0, component_count * sizeof(float));
        return;
    }
    if (sampler->keyframe_count == 1 || time_seconds <= sampler->keyframes[0])
    {
        memcpy(out_values, sampler->values, component_count * sizeof(float));
        return;
    }
    if (time_seconds >= sampler->keyframes[sampler->keyframe_count - 1])
    {
        memcpy(
            out_values,
            &sampler->values[(sampler->keyframe_count - 1) * component_count],
            component_count * sizeof(float)
        );
        return;
    }

    base_index = find_keyframe_index(sampler, time_seconds);
    next_index = base_index + 1;
    frame_a = sampler->keyframes[base_index];
    frame_b = sampler->keyframes[next_index];
    a = &sampler->values[base_index * component_count];
    b = &sampler->values[next_index * component_count];
    t = frame_b > frame_a ? (time_seconds - frame_a) / (frame_b - frame_a) : 0.0f;

    if (sampler->interpolation == ANIMATION_INTERPOLATION_STEP)
    {
        memcpy(out_values, a, component_count * sizeof(float));
        return;
    }
    if (sampler->interpolation == ANIMATION_INTERPOLATION_CUBIC)
    {
        size_t value_components = component_count / 3;
        const float* cubic_a = a + value_components;
        const float* cubic_b = b + value_components;
        memcpy(out_values, cubic_a, value_components * sizeof(float));
        for (i = 0; i < value_components; ++i)
        {
            out_values[i] = lerpf(cubic_a[i], cubic_b[i], t);
        }
        return;
    }

    for (i = 0; i < component_count; ++i)
    {
        out_values[i] = lerpf(a[i], b[i], t);
    }
}

void UpdateSceneAnimation(Scene* scene, size_t animation_index, float time_seconds, bool loop)
{
    SceneAnimation* animation;
    size_t channel_index;

    if (scene == NULL || animation_index >= scene->animation_count)
    {
        return;
    }

    ResetScenePose(scene);
    animation = &scene->animations[animation_index];
    if (animation->duration > 0.0f && loop)
    {
        time_seconds = fmodf(time_seconds, animation->duration);
        if (time_seconds < 0.0f)
        {
            time_seconds += animation->duration;
        }
    }
    else
    {
        time_seconds = clampf(time_seconds, 0.0f, animation->duration);
    }

    for (channel_index = 0; channel_index < animation->channel_count; ++channel_index)
    {
        AnimationChannel* channel = &animation->channels[channel_index];
        SceneNode* node;
        float sampled[12] = {0};

        if (channel->node_index < 0 || channel->node_index >= (int)scene->node_count)
        {
            continue;
        }

        node = &scene->nodes[channel->node_index];
        sample_animation_vec(&channel->sampler, time_seconds, sampled);

        switch (channel->path)
        {
        case ANIMATION_PATH_TRANSLATION:
            node->local_translation = vec3(sampled[0], sampled[1], sampled[2]);
            break;
        case ANIMATION_PATH_ROTATION:
            node->local_rotation = quat_slerp(node->local_rotation, quat(sampled[0], sampled[1], sampled[2], sampled[3]), 1.0f);
            break;
        case ANIMATION_PATH_SCALE:
            node->local_scale = vec3(sampled[0], sampled[1], sampled[2]);
            break;
        case ANIMATION_PATH_WEIGHTS:
            break;
        }
    }

    UpdateSceneWorldTransforms(scene);
}

static void update_node_world_transform(Scene* scene, int node_index)
{
    SceneNode* node;
    size_t child_index;

    if (node_index < 0 || node_index >= (int)scene->node_count)
    {
        return;
    }

    node = &scene->nodes[node_index];
    apply_node_local_transform(node);
    if (node->parent_index >= 0)
    {
        node->world_matrix = mat4_mul(scene->nodes[node->parent_index].world_matrix, node->local_matrix);
    }
    else
    {
        node->world_matrix = node->local_matrix;
    }

    for (child_index = 0; child_index < node->child_count; ++child_index)
    {
        update_node_world_transform(scene, node->child_indices[child_index]);
    }
}

void UpdateSceneWorldTransforms(Scene* scene)
{
    size_t root_index;

    if (scene == NULL)
    {
        return;
    }

    if (scene->scene_root_count > 0)
    {
        for (root_index = 0; root_index < scene->scene_root_count; ++root_index)
        {
            update_node_world_transform(scene, scene->scene_roots[root_index]);
        }
    }
    else
    {
        for (root_index = 0; root_index < scene->node_count; ++root_index)
        {
            if (scene->nodes[root_index].parent_index < 0)
            {
                update_node_world_transform(scene, (int)root_index);
            }
        }
    }
}

void ReleaseScene(Scene scene)
{
    size_t i;
    size_t j;

    for (i = 0; i < scene.texture_count; ++i)
    {
        ReleaseTexture(scene.textures[i]);
    }
    free(scene.textures);

    for (i = 0; i < scene.material_count; ++i)
    {
        free(scene.materials[i].name);
    }
    free(scene.materials);

    for (i = 0; i < scene.mesh_count; ++i)
    {
        for (j = 0; j < scene.meshes[i].primitive_count; ++j)
        {
            ReleaseMesh(scene.meshes[i].primitives[j].mesh);
            free(scene.meshes[i].primitives[j].name);
        }
        free(scene.meshes[i].primitives);
        free(scene.meshes[i].name);
    }
    free(scene.meshes);

    for (i = 0; i < scene.node_count; ++i)
    {
        free(scene.nodes[i].name);
        free(scene.nodes[i].child_indices);
    }
    free(scene.nodes);
    free(scene.scene_roots);

    for (i = 0; i < scene.skin_count; ++i)
    {
        free(scene.skins[i].name);
        free(scene.skins[i].joints);
        free(scene.skins[i].inverse_bind_matrices);
    }
    free(scene.skins);

    for (i = 0; i < scene.animation_count; ++i)
    {
        for (j = 0; j < scene.animations[i].channel_count; ++j)
        {
            free(scene.animations[i].channels[j].sampler.keyframes);
            free(scene.animations[i].channels[j].sampler.values);
        }
        free(scene.animations[i].channels);
        free(scene.animations[i].name);
    }
    free(scene.animations);
    free(scene.source_path);
}
