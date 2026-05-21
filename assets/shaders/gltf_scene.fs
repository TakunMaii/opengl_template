#version 330 core

in vec3 vWorldPosition;
in vec3 vWorldNormal;
in vec3 vViewNormal;
in vec2 vTexcoord0;
in vec2 vTexcoord1;
in vec4 vColor;
in vec4 vTangent;

uniform vec3 uViewPosition;
uniform vec3 uLightDirection;
uniform vec3 uAmbientColor;

uniform vec4 uBaseColorFactor;
uniform float uMetallicFactor;
uniform float uRoughnessFactor;
uniform vec3 uEmissiveFactor;
uniform float uAlphaCutoff;
uniform int uAlphaMode;
uniform int uUnlit;
uniform int uRenderMode;

uniform int uHasBaseColorTexture;
uniform int uHasMetallicRoughnessTexture;
uniform int uHasNormalTexture;
uniform int uHasOcclusionTexture;
uniform int uHasEmissiveTexture;

uniform sampler2D uBaseColorMap;
uniform sampler2D uMetallicRoughnessMap;
uniform sampler2D uNormalMap;
uniform sampler2D uOcclusionMap;
uniform sampler2D uEmissiveMap;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 NormalColor;

vec2 uv_for_set(int texcoordSet)
{
    return texcoordSet == 1 ? vTexcoord1 : vTexcoord0;
}

vec3 sample_normal()
{
    vec3 normal = normalize(vWorldNormal);
    if (uHasNormalTexture == 0)
    {
        return normal;
    }

    vec3 tangent = normalize(vTangent.xyz);
    vec3 bitangent = normalize(cross(normal, tangent) * vTangent.w);
    mat3 tbn = mat3(tangent, bitangent, normal);
    vec3 mapped = texture(uNormalMap, vTexcoord0).xyz * 2.0 - 1.0;
    return normalize(tbn * mapped);
}

void main()
{
    vec4 baseColor = uBaseColorFactor * vColor;
    vec3 metallicRoughness = vec3(1.0, uRoughnessFactor, uMetallicFactor);
    vec3 normal = sample_normal();
    vec3 lightDir = normalize(-uLightDirection);
    vec3 viewDir = normalize(uViewPosition - vWorldPosition);
    vec3 halfDir = normalize(lightDir + viewDir);
    float ndotl = max(dot(normal, lightDir), 0.0);
    float ndotv = max(dot(normal, viewDir), 0.0);
    vec3 emissive = uEmissiveFactor;
    float occlusion = 1.0;

    if (uHasBaseColorTexture != 0)
    {
        baseColor *= texture(uBaseColorMap, vTexcoord0);
    }
    if (uHasMetallicRoughnessTexture != 0)
    {
        vec4 mr = texture(uMetallicRoughnessMap, vTexcoord0);
        metallicRoughness.g *= mr.g;
        metallicRoughness.b *= mr.b;
    }
    if (uHasOcclusionTexture != 0)
    {
        occlusion = texture(uOcclusionMap, vTexcoord0).r;
    }
    if (uHasEmissiveTexture != 0)
    {
        emissive *= texture(uEmissiveMap, vTexcoord0).rgb;
    }

    if (uAlphaMode == 1 && baseColor.a < uAlphaCutoff)
    {
        discard;
    }

    if (uUnlit != 0)
    {
        FragColor = vec4(baseColor.rgb + emissive, baseColor.a);
        NormalColor = vec4(normalize(vViewNormal) * 0.5 + 0.5, 1.0);
        return;
    }

    if (uRenderMode == 1)
    {
        float wrapped = clamp(ndotl * 0.45 + 0.4, 0.0, 1.0);
        float shadowBand = smoothstep(0.55, 0.62, wrapped);
        float specularBase = pow(max(dot(normal, halfDir), 0.0), 48.0);
        float highlight = smoothstep(0.70, 0.76, specularBase) * 0.15;
        float rim = smoothstep(0.60, 0.78, 1.0 - ndotv) * mix(1.0, 0.35, shadowBand) * 0.14;
        vec3 shadowed = mix(baseColor.rgb * vec3(0.68, 0.74, 0.90), baseColor.rgb, shadowBand);
        vec3 color = shadowed + baseColor.rgb * uAmbientColor + vec3(highlight) + vec3(rim);
        color += emissive;
        color *= occlusion;
        FragColor = vec4(clamp(color, 0.0, 1.0), baseColor.a);
        NormalColor = vec4(normalize(vViewNormal) * 0.5 + 0.5, 1.0);
        return;
    }

    {
        float roughness = clamp(metallicRoughness.g, 0.04, 1.0);
        float metallic = clamp(metallicRoughness.b, 0.0, 1.0);
        vec3 diffuseColor = baseColor.rgb * (1.0 - metallic);
        vec3 f0 = mix(vec3(0.04), baseColor.rgb, metallic);
        float specPower = mix(128.0, 8.0, roughness);
        float specular = pow(max(dot(normal, halfDir), 0.0), specPower);
        vec3 fresnel = f0 + (1.0 - f0) * pow(1.0 - max(dot(halfDir, viewDir), 0.0), 5.0);
        vec3 color = diffuseColor * ndotl + fresnel * specular;
        color += diffuseColor * uAmbientColor;
        color += emissive;
        color *= occlusion;
        FragColor = vec4(clamp(color, 0.0, 1.0), baseColor.a);
        NormalColor = vec4(normalize(vViewNormal) * 0.5 + 0.5, 1.0);
    }
}
