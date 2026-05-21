#version 330 core

in vec3 vNormal;
in vec4 vColor;

uniform vec3 uLightDirection;
out vec4 FragColor;

void main()
{
    float light = max(dot(normalize(vNormal), normalize(-uLightDirection)), 0.15);
    vec3 litColor = vColor.rgb * light;
    FragColor = vec4(litColor, vColor.a);
}
