#include "renderer.h"

#include "render/color.h"
#include "utils/log.h"
#include <glad/glad.h>
#include <stdio.h>
#include <string.h>

static const int MAX_JOINT_UNIFORMS = SCENE_MAX_SKIN_JOINTS;

static void compute_skin_matrices(const Scene* scene, int skin_index, Mat4* output_matrices, int* count_out)
{
    size_t i;
    Skin* skin;

    if (count_out == NULL)
    {
        return;
    }

    *count_out = 0;
    if (scene == NULL || skin_index < 0 || skin_index >= (int)scene->skin_count)
    {
        return;
    }

    skin = &scene->skins[skin_index];
    for (i = 0; i < skin->joint_count && i < (size_t)MAX_JOINT_UNIFORMS; ++i)
    {
        int node_index = skin->joints[i];
        if (node_index < 0 || node_index >= (int)scene->node_count)
        {
            output_matrices[i] = mat4_identity();
            continue;
        }

        output_matrices[i] = mat4_mul(scene->nodes[node_index].world_matrix, skin->inverse_bind_matrices[i]);
        *count_out = (int)(i + 1);
    }
}

static void set_skin_uniforms(Shader shader, const Scene* scene, int skin_index)
{
    Mat4 matrices[SCENE_MAX_SKIN_JOINTS];
    int matrix_count = 0;
    int i;
    char uniform_name[64];

    compute_skin_matrices(scene, skin_index, matrices, &matrix_count);
    SetShaderInt(shader, "uJointCount", matrix_count);
    for (i = 0; i < matrix_count; ++i)
    {
        snprintf(uniform_name, sizeof(uniform_name), "uJointMatrices[%d]", i);
        SetShaderMat4(shader, uniform_name, matrices[i]);
    }
}

static void set_material_uniforms(Renderer* renderer, const Material* material)
{
    Texture* base_color_texture = material->base_color_texture.texture;
    Texture* metallic_roughness_texture = material->metallic_roughness_texture.texture;
    Texture* normal_texture = material->normal_texture.texture;
    Texture* occlusion_texture = material->occlusion_texture.texture;
    Texture* emissive_texture = material->emissive_texture.texture;

    SetShaderVec4(renderer->scene_shader, "uBaseColorFactor", material->base_color_factor);
    SetShaderFloat(renderer->scene_shader, "uMetallicFactor", material->metallic_factor);
    SetShaderFloat(renderer->scene_shader, "uRoughnessFactor", material->roughness_factor);
    SetShaderVec3(renderer->scene_shader, "uEmissiveFactor", material->emissive_factor);
    SetShaderFloat(renderer->scene_shader, "uAlphaCutoff", material->alpha_cutoff);
    SetShaderInt(renderer->scene_shader, "uAlphaMode", (int)material->alpha_mode);
    SetShaderInt(renderer->scene_shader, "uUnlit", material->unlit ? 1 : 0);
    SetShaderInt(renderer->scene_shader, "uRenderMode", (int)renderer->settings.render_mode);

    SetShaderInt(renderer->scene_shader, "uHasBaseColorTexture", material->base_color_texture.enabled && base_color_texture != NULL);
    SetShaderInt(renderer->scene_shader, "uHasMetallicRoughnessTexture", material->metallic_roughness_texture.enabled && metallic_roughness_texture != NULL);
    SetShaderInt(renderer->scene_shader, "uHasNormalTexture", material->normal_texture.enabled && normal_texture != NULL);
    SetShaderInt(renderer->scene_shader, "uHasOcclusionTexture", material->occlusion_texture.enabled && occlusion_texture != NULL);
    SetShaderInt(renderer->scene_shader, "uHasEmissiveTexture", material->emissive_texture.enabled && emissive_texture != NULL);

    BindTexture(base_color_texture != NULL ? *base_color_texture : renderer->white_texture, 0);
    BindTexture(metallic_roughness_texture != NULL ? *metallic_roughness_texture : renderer->white_texture, 1);
    BindTexture(normal_texture != NULL ? *normal_texture : renderer->normal_texture, 2);
    BindTexture(occlusion_texture != NULL ? *occlusion_texture : renderer->white_texture, 3);
    BindTexture(emissive_texture != NULL ? *emissive_texture : renderer->black_texture, 4);
}

RendererSettings DefaultRendererSettings(void)
{
    RendererSettings settings;
    memset(&settings, 0, sizeof(settings));
    settings.light_direction = normalize3(vec3(-0.5f, -1.0f, -0.4f));
    settings.ambient_color = vec3(0.06f, 0.07f, 0.08f);
    settings.clear_color = vec4(0.87f, 0.92f, 0.98f, 1.0f);
    settings.outline_color = vec4(0.08f, 0.09f, 0.13f, 1.0f);
    settings.normal_edge_threshold = 0.18f;
    settings.depth_edge_threshold = 0.0035f;
    settings.outline_strength = 1.0f;
    settings.render_mode = RENDER_MODE_TOON;
    settings.enable_outline = true;
    return settings;
}

Renderer CreateRenderer(int width, int height, RendererSettings settings)
{
    Renderer renderer;
    FramebufferDesc desc;

    memset(&renderer, 0, sizeof(renderer));
    renderer.settings = settings;
    renderer.scene_shader = LoadShader("assets/shaders/gltf_scene.vs", "assets/shaders/gltf_scene.fs");
    renderer.composite_shader = LoadShader("assets/shaders/postprocess_outline.vs", "assets/shaders/postprocess_outline.fs");
    renderer.screen_quad = GenerateRectangle2D(vec2(0.0f, 0.0f), vec2(2.0f, 2.0f), color(1.0f, 1.0f, 1.0f, 1.0f));
    renderer.white_texture = CreateSolidTexture(color(1.0f, 1.0f, 1.0f, 1.0f));
    renderer.normal_texture = CreateSolidTexture(color(0.5f, 0.5f, 1.0f, 1.0f));
    renderer.black_texture = CreateSolidTexture(color(0.0f, 0.0f, 0.0f, 1.0f));

    desc = DefaultFramebufferDesc(width, height);
    desc.color_attachment_count = 2;
    desc.color_attachments[0] = DefaultColorAttachmentDesc(FRAMEBUFFER_ATTACHMENT_RGBA8);
    desc.color_attachments[1] = DefaultColorAttachmentDesc(FRAMEBUFFER_ATTACHMENT_RGBA8);
    desc.has_depth_attachment = true;
    desc.depth_attachment = DefaultDepthAttachmentDesc();
    renderer.framebuffer = CreateFramebuffer(desc);

    return renderer;
}

void ResizeRenderer(Renderer* renderer, int width, int height)
{
    if (renderer == NULL)
    {
        return;
    }
    ResizeFramebuffer(&renderer->framebuffer, width, height);
}

void RenderScene(Renderer* renderer, const Scene* scene, Camera3D camera)
{
    size_t node_index;
    int width = renderer->framebuffer.width;
    int height = renderer->framebuffer.height;

    BindFramebuffer(renderer->framebuffer);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glClearColor(renderer->settings.clear_color.x, renderer->settings.clear_color.y, renderer->settings.clear_color.z, renderer->settings.clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    BeginShaderMode(renderer->scene_shader);
    SetShaderMat4(renderer->scene_shader, "uView", camera.view);
    SetShaderMat4(renderer->scene_shader, "uProjection", camera.projection);
    SetShaderVec3(renderer->scene_shader, "uViewPosition", camera.position);
    SetShaderVec3(renderer->scene_shader, "uLightDirection", renderer->settings.light_direction);
    SetShaderVec3(renderer->scene_shader, "uAmbientColor", renderer->settings.ambient_color);
    SetShaderInt(renderer->scene_shader, "uBaseColorMap", 0);
    SetShaderInt(renderer->scene_shader, "uMetallicRoughnessMap", 1);
    SetShaderInt(renderer->scene_shader, "uNormalMap", 2);
    SetShaderInt(renderer->scene_shader, "uOcclusionMap", 3);
    SetShaderInt(renderer->scene_shader, "uEmissiveMap", 4);

    for (node_index = 0; node_index < scene->node_count; ++node_index)
    {
        const SceneNode* node = &scene->nodes[node_index];
        if (node->mesh_index < 0 || node->mesh_index >= (int)scene->mesh_count)
        {
            continue;
        }

        {
            const SceneMesh* mesh = &scene->meshes[node->mesh_index];
            size_t primitive_index;

            for (primitive_index = 0; primitive_index < mesh->primitive_count; ++primitive_index)
            {
                const Primitive* primitive = &mesh->primitives[primitive_index];
                const Material* material = primitive->material;

                SetShaderMat4(renderer->scene_shader, "uModel", node->world_matrix);
                SetShaderInt(renderer->scene_shader, "uSkinned", primitive->skinned ? 1 : 0);
                if (primitive->skinned)
                {
                    set_skin_uniforms(renderer->scene_shader, scene, primitive->skin_index);
                }
                else
                {
                    SetShaderInt(renderer->scene_shader, "uJointCount", 0);
                }

                if (material->double_sided)
                {
                    glDisable(GL_CULL_FACE);
                }
                else
                {
                    glEnable(GL_CULL_FACE);
                    glCullFace(GL_BACK);
                }

                if (material->alpha_mode == MATERIAL_ALPHA_BLEND)
                {
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                }
                else
                {
                    glDisable(GL_BLEND);
                }

                set_material_uniforms(renderer, material);
                RenderMesh(primitive->mesh);
                UnbindTexture(4);
                UnbindTexture(3);
                UnbindTexture(2);
                UnbindTexture(1);
                UnbindTexture(0);
            }
        }
    }
    EndShaderMode();

    BindDefaultFramebuffer();
    glViewport(0, 0, width, height);
    glDisable(GL_DEPTH_TEST);
    glClearColor(renderer->settings.clear_color.x, renderer->settings.clear_color.y, renderer->settings.clear_color.z, renderer->settings.clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    BeginShaderMode(renderer->composite_shader);
    SetShaderInt(renderer->composite_shader, "uSceneColor", 0);
    SetShaderInt(renderer->composite_shader, "uSceneNormal", 1);
    SetShaderInt(renderer->composite_shader, "uSceneDepth", 2);
    SetShaderVec2(renderer->composite_shader, "uTexelSize", vec2(1.0f / (float)width, 1.0f / (float)height));
    SetShaderVec4(renderer->composite_shader, "uOutlineColor", renderer->settings.outline_color);
    SetShaderFloat(renderer->composite_shader, "uNormalEdgeThreshold", renderer->settings.normal_edge_threshold);
    SetShaderFloat(renderer->composite_shader, "uDepthEdgeThreshold", renderer->settings.depth_edge_threshold);
    SetShaderFloat(renderer->composite_shader, "uOutlineStrength", renderer->settings.enable_outline ? renderer->settings.outline_strength : 0.0f);
    BindTexture(GetFramebufferColorAttachment(renderer->framebuffer, 0), 0);
    BindTexture(GetFramebufferColorAttachment(renderer->framebuffer, 1), 1);
    BindTexture(GetFramebufferDepthAttachment(renderer->framebuffer), 2);
    RenderMesh(renderer->screen_quad);
    UnbindTexture(2);
    UnbindTexture(1);
    UnbindTexture(0);
    EndShaderMode();

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void ReleaseRenderer(Renderer renderer)
{
    ReleaseFramebuffer(renderer.framebuffer);
    ReleaseMesh(renderer.screen_quad);
    ReleaseTexture(renderer.white_texture);
    ReleaseTexture(renderer.normal_texture);
    ReleaseTexture(renderer.black_texture);
    ReleaseShader(renderer.scene_shader);
    ReleaseShader(renderer.composite_shader);
}
