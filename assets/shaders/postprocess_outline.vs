#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexcoord;

out vec2 vTexcoord;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    vTexcoord = vec2(aTexcoord.x, 1.0 - aTexcoord.y);
}
