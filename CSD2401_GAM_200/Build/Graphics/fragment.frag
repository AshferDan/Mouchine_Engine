#version 450 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec4 uColor;
uniform vec2 uTiling;

void main() {
    vec4 tex = texture(uTexture, TexCoord * uTiling);
    FragColor = tex * uColor;
}
