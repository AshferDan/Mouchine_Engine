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
uniform int uIsLine;

void main() {
    vec4 baseColor = (uUseBatching == 1) ? VertColor : uColor;
    vec2 tiling = (uUseBatching == 1) ? UVScale : uTiling;
    vec2 offset = (uUseBatching == 1) ? UVOffset : uTexOffset;

    if (uIsLine == 1) {
        FragColor = uColor; // just draw solid color
        return;
    }

   if(uHasTexture == 1) {
      vec2 finalTexCoord = TexCoord * uTiling + offset; 
        
        vec4 tex = texture(uTexture, finalTexCoord);
        baseColor *= tex;
    }

    if(uIsCircle == 1) {
        float dist = dot(CircleCoord, CircleCoord); // distance squared from center
        float edge = 0.01; // width of the edge fade
        float alpha = 1.0 - smoothstep(1.0 - edge, 1.0, dist);
        baseColor.a *= alpha;
        if(alpha <= 0.0) discard;
    }

    FragColor = baseColor;
}