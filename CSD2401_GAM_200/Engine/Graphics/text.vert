#version 450 core
layout (location = 0) in vec4 vertex;

uniform mat4 uMVP;
out vec2 TexCoords;

void main() {
	gl_Position = uMVP * vec4(vertex.xy, 0.0, 1.0);
	TexCoords = vertex.zw;
}