#version 450 core
in vec2 TexCoord;
in vec2 CircleCoord;
in vec4 VertColor;     
in vec2 UVScale;       
in vec2 UVOffset;      
out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec4 uColor;        
uniform vec2 uTiling;       
uniform vec2 uTexOffset;    
uniform int uUseBatching;   

uniform int uIsCircle;
uniform int uHasTexture;

void main() {
    vec4 baseColor = (uUseBatching == 1) ? VertColor : uColor;
    vec2 tiling = (uUseBatching == 1) ? UVScale : uTiling;
    vec2 offset = (uUseBatching == 1) ? UVOffset : uTexOffset;

    if(uHasTexture == 1) {
        vec2 finalTexCoord = TexCoord * ((uUseBatching == 1) ? UVScale : uTiling)
                             + ((uUseBatching == 1) ? UVOffset : uTexOffset);
        vec4 tex = texture(uTexture, finalTexCoord);
        baseColor *= tex;
    }

    if (uIsCircle == 1) {
        float distSq = dot(CircleCoord, CircleCoord);
        if (distSq > 1.0) discard;  // Hard edge, no smoothstep!
    }

    FragColor = baseColor;
}