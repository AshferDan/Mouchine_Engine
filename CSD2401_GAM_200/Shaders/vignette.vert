#version 450 core

layout(location = 0) in vec3 aPos;

out vec2 fragTexCoord;

void main() {
	fragTexCoord = aPos.xy * 0.5 + 0.5; // Convert from [-1, 1] to [0, 1]
	gl_Position = vec4(aPos, 1.0);
}