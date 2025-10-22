#version 450 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTex;
layout(location = 2) in vec4 aColor;      
layout(location = 3) in vec2 aUVScale;    
layout(location = 4) in vec2 aUVOffset;   

out vec2 TexCoord;
out vec2 CircleCoord;
out vec4 VertColor;                        
out vec2 UVScale;                          
out vec2 UVOffset;                         

uniform mat4 uMVP;

void main() {
    TexCoord = aTex;
    CircleCoord = aTex * 2.0 - 1.0;
    VertColor = aColor;
    UVScale = aUVScale;
    UVOffset = aUVOffset;
    gl_Position = uMVP * vec4(aPos, 1.0);
}