#version 330 core

in vec3 vNormal;
in vec2 vTexcoord;
in vec4 vColor;

uniform vec3 uLightDirection;
uniform sampler2D uDiffuseMap;
out vec4 FragColor;

void main()
{
    float light = max(dot(normalize(vNormal), normalize(-uLightDirection)), 0.15);
    vec4 texColor = texture(uDiffuseMap, vTexcoord);
    vec4 baseColor = texColor * vColor;
    vec3 litColor = baseColor.rgb * light;
    FragColor = vec4(litColor, baseColor.a);
}
