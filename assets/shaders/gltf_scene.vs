#version 330 core

#define MAX_JOINTS 128

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexcoord0;
layout (location = 3) in vec2 aTexcoord1;
layout (location = 4) in vec4 aTangent;
layout (location = 5) in vec4 aColor;
layout (location = 6) in vec4 aJoints;
layout (location = 7) in vec4 aWeights;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform int uSkinned;
uniform int uJointCount;
uniform mat4 uJointMatrices[MAX_JOINTS];

out vec3 vWorldPosition;
out vec3 vWorldNormal;
out vec3 vViewNormal;
out vec2 vTexcoord0;
out vec2 vTexcoord1;
out vec4 vColor;
out vec4 vTangent;

mat4 skin_matrix()
{
    if (uSkinned == 0 || uJointCount <= 0)
    {
        return mat4(1.0);
    }

    mat4 skin = mat4(0.0);
    ivec4 joints = ivec4(aJoints);

    if (joints.x < uJointCount) skin += uJointMatrices[joints.x] * aWeights.x;
    if (joints.y < uJointCount) skin += uJointMatrices[joints.y] * aWeights.y;
    if (joints.z < uJointCount) skin += uJointMatrices[joints.z] * aWeights.z;
    if (joints.w < uJointCount) skin += uJointMatrices[joints.w] * aWeights.w;
    return skin;
}

void main()
{
    mat4 skin = skin_matrix();
    vec4 localPosition = skin * vec4(aPos, 1.0);
    vec3 localNormal = normalize(mat3(skin) * aNormal);
    vec3 localTangent = normalize(mat3(skin) * aTangent.xyz);
    vec4 worldPosition = uModel * localPosition;
    mat3 normalMatrix = mat3(transpose(inverse(uModel)));
    mat3 viewNormalMatrix = mat3(transpose(inverse(uView * uModel)));

    gl_Position = uProjection * uView * worldPosition;
    vWorldPosition = worldPosition.xyz;
    vWorldNormal = normalize(normalMatrix * localNormal);
    vViewNormal = normalize(viewNormalMatrix * localNormal);
    vTexcoord0 = aTexcoord0;
    vTexcoord1 = aTexcoord1;
    vColor = aColor;
    vTangent = vec4(normalize(normalMatrix * localTangent), aTangent.w);
}
