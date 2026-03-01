#version 450 core

in vec2 fragTexCoord;
out vec4 fragColor;

uniform sampler2D uSceneTexture;
uniform float uInnerRadius;   // where vignette starts (e.g. 0.3)
uniform float uOuterRadius;   // where vignette is fully dark (e.g. 0.75)
uniform vec4  uVignetteColor; // color of the vignette (e.g. black = 0,0,0,1)
uniform float uIntensity;     // overall strength, 0.0 = off, 1.0 = full

void main()
{
    // distance from center (0.5, 0.5) in UV space
    vec2 center = fragTexCoord - 0.5;

    // correct for non-square screens so vignette is circular not oval
    center.x *= 1.77778; // 16/9, adjust if your resolution ratio differs

    float dist = length(center);

    // smoothstep gives a soft gradient between inner and outer radius
    float vignette = 1.0 - smoothstep(uInnerRadius, uOuterRadius, dist);

    // blend between vignette color and scene color based on vignette strength
    vec4 scene = texture(uSceneTexture, fragTexCoord);
    vec3 blended = mix(uVignetteColor.rgb, scene.rgb, vignette * uIntensity + (1.0 - uIntensity));

    fragColor = vec4(blended, 1.0);
}