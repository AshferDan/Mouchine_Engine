#version 450 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTex;
layout(location = 2) in vec4 aColor;      
layout(location = 3) in vec2 aUVScale;    
layout(location = 4) in vec2 aUVOffset;   
layout(location = 5) in vec3 aTransform;
layout(location = 6) in vec2 aScale;
layout(location = 7) in vec2 aTileCount;  // NEW

out vec2 TexCoord;
out vec2 CircleCoord;
out vec4 VertColor;                        
out vec2 UVScale;                          
out vec2 UVOffset;
out vec2 TileCount;                        // NEW

uniform mat4 uMVP;
uniform int uUseBatching;

void main() {
    vec2 finalPos;
    if(uUseBatching == 1) {
        float worldX = aTransform.x;
        float worldY = aTransform.y;
        float rotation = aTransform.z;
        float scaleX = aScale.x;
        float scaleY = aScale.y;
        float cosR = cos(rotation);
        float sinR = sin(rotation);
        mat2 rotMatrix = mat2(
            cosR, sinR,
            -sinR, cosR
        );
        vec2 scaledPos = aPos.xy * aScale;
        vec2 rotatedPos = rotMatrix * scaledPos;
        finalPos = rotatedPos + aTransform.xy;
    } else {
        finalPos = aPos.xy;
    }

    gl_Position = uMVP * vec4(finalPos, 0.0, 1.0);
    TexCoord    = aTex;
    CircleCoord = aTex * 2.0 - 1.0;
    VertColor   = aColor;
    UVScale     = aUVScale;
    UVOffset    = aUVOffset;
    TileCount   = (uUseBatching == 1) ? aTileCount : vec2(1.0); // NEW
}