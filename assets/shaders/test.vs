#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexcoord;
layout (location = 3) in vec4 aColor;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vNormal;
out vec4 vColor;

void main()
{
    mat3 normalMatrix = mat3(transpose(inverse(uModel)));
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
    vNormal = normalize(normalMatrix * aNormal);
    vColor = aColor;
}
