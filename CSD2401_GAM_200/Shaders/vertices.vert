#version 450 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTex;

out vec2 TexCoord;
out vec2 CircleCoord;

uniform mat4 uMVP;

void main() {
    TexCoord = aTex;
    CircleCoord = aTex * 2.0 - 1.0;
    gl_Position = uMVP * vec4(aPos, 1.0);
}