#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexcoord;
layout (location = 3) in vec4 aColor;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vWorldPosition;
out vec3 vWorldNormal;
out vec3 vViewNormal;
out vec2 vTexcoord;
out vec4 vColor;

void main()
{
    vec4 worldPosition = uModel * vec4(aPos, 1.0);
    mat3 normalMatrix = mat3(transpose(inverse(uModel)));
    mat3 viewNormalMatrix = mat3(transpose(inverse(uView * uModel)));

    gl_Position = uProjection * uView * worldPosition;
    vWorldPosition = worldPosition.xyz;
    vWorldNormal = normalize(normalMatrix * aNormal);
    vViewNormal = normalize(viewNormalMatrix * aNormal);
    vTexcoord = aTexcoord;
    vColor = aColor;
}
