#version 330 core

in vec3 vWorldPosition;
in vec3 vWorldNormal;
in vec3 vViewNormal;
in vec2 vTexcoord;
in vec4 vColor;

uniform vec3 uViewPosition;
uniform vec3 uLightDirection;
uniform sampler2D uDiffuseMap;

uniform float uShadowThreshold;
uniform float uShadowSoftness;
uniform float uShadowStrength;
uniform vec3 uShadowTint;

uniform float uHighlightThreshold;
uniform float uHighlightSoftness;
uniform float uHighlightStrength;
uniform vec3 uHighlightColor;

uniform float uRimThreshold;
uniform float uRimSoftness;
uniform float uRimStrength;
uniform vec3 uRimColor;

uniform vec3 uAmbientColor;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 NormalColor;

void main()
{
    vec4 texColor = texture(uDiffuseMap, vTexcoord);
    vec4 baseColor = texColor * vColor;
    vec3 normal = normalize(vWorldNormal);
    vec3 lightDir = normalize(-uLightDirection);
    vec3 viewDir = normalize(uViewPosition - vWorldPosition);
    vec3 halfDir = normalize(lightDir + viewDir);
    float ndotl = dot(normal, lightDir);
    float wrappedDiffuse = clamp(ndotl * 0.45 + 0.4, 0.0, 1.0);
    float litBand = smoothstep(
        uShadowThreshold - uShadowSoftness,
        uShadowThreshold + uShadowSoftness,
        wrappedDiffuse
    );
    vec3 shadowedBase = mix(baseColor.rgb, baseColor.rgb * uShadowTint, uShadowStrength);
    vec3 diffuseColor = mix(shadowedBase, baseColor.rgb, litBand);

    float specularBase = max(dot(normal, halfDir), 0.0);
    specularBase = pow(specularBase, 48.0);
    float highlightMask = smoothstep(
        uHighlightThreshold - uHighlightSoftness,
        uHighlightThreshold + uHighlightSoftness,
        specularBase
    );
    vec3 highlight = uHighlightColor * highlightMask * uHighlightStrength;

    float rimBase = 1.0 - max(dot(normal, viewDir), 0.0);
    float rimMask = smoothstep(
        uRimThreshold - uRimSoftness,
        uRimThreshold + uRimSoftness,
        rimBase
    );
    rimMask *= mix(1.0, 0.35, litBand);
    vec3 rimLight = uRimColor * rimMask * uRimStrength;

    vec3 finalColor = diffuseColor;
    finalColor += baseColor.rgb * uAmbientColor;
    finalColor += highlight;
    finalColor += rimLight;
    finalColor *= 0.92;

    FragColor = vec4(clamp(finalColor, 0.0, 1.0), baseColor.a);
    NormalColor = vec4(normalize(vViewNormal) * 0.5 + 0.5, 1.0);
}
