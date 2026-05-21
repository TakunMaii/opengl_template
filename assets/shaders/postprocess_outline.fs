#version 330 core

in vec2 vTexcoord;

uniform sampler2D uSceneColor;
uniform sampler2D uSceneNormal;
uniform sampler2D uSceneDepth;
uniform vec2 uTexelSize;
uniform vec4 uOutlineColor;
uniform float uNormalEdgeThreshold;
uniform float uDepthEdgeThreshold;
uniform float uOutlineStrength;

out vec4 FragColor;

vec3 decode_normal(vec3 encodedNormal)
{
    return normalize(encodedNormal * 2.0 - 1.0);
}

float sample_depth_edge(vec2 uv)
{
    float centerDepth = texture(uSceneDepth, uv).r;
    float depthDifference = 0.0;
    vec2 offsets[4] = vec2[](
        vec2(-uTexelSize.x, 0.0),
        vec2(uTexelSize.x, 0.0),
        vec2(0.0, -uTexelSize.y),
        vec2(0.0, uTexelSize.y)
    );
    int i;

    for (i = 0; i < 4; ++i)
    {
        float sampleDepth = texture(uSceneDepth, uv + offsets[i]).r;
        depthDifference = max(depthDifference, abs(sampleDepth - centerDepth));
    }

    return smoothstep(
        uDepthEdgeThreshold * 0.5,
        uDepthEdgeThreshold,
        depthDifference
    );
}

float sample_normal_edge(vec2 uv)
{
    vec3 centerNormal = decode_normal(texture(uSceneNormal, uv).rgb);
    float normalDifference = 0.0;
    vec2 offsets[4] = vec2[](
        vec2(-uTexelSize.x, 0.0),
        vec2(uTexelSize.x, 0.0),
        vec2(0.0, -uTexelSize.y),
        vec2(0.0, uTexelSize.y)
    );
    int i;

    for (i = 0; i < 4; ++i)
    {
        vec3 sampleNormal = decode_normal(texture(uSceneNormal, uv + offsets[i]).rgb);
        normalDifference = max(normalDifference, 1.0 - dot(centerNormal, sampleNormal));
    }

    return smoothstep(
        uNormalEdgeThreshold * 0.5,
        uNormalEdgeThreshold,
        normalDifference
    );
}

void main()
{
    vec4 sceneColor = texture(uSceneColor, vTexcoord);
    float depthEdge = sample_depth_edge(vTexcoord);
    float normalEdge = sample_normal_edge(vTexcoord);
    float outlineMask = clamp(max(depthEdge, normalEdge) * uOutlineStrength, 0.0, 1.0);

    FragColor = mix(sceneColor, uOutlineColor, outlineMask * uOutlineColor.a);
}
