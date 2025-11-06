#version 450 core
in vec4 VertColor;
out vec4 FragColor;

uniform vec4 uColor;
uniform int uUseBatching;
uniform int uIsLine;

void main() {
    // In wireframe mode, just use color (no textures)
    vec4 baseColor = (uUseBatching == 1) ? VertColor : uColor;
    
    if (uIsLine == 1) {
        FragColor = uColor;
    } else {
        // Wireframe color - bright for visibility
        FragColor = vec4(0.0, 1.0, 0.0, 1.0);  // Green wireframe
    }
}