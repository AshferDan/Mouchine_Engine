#version 450 core
in vec2 TexCoord;
in vec2 CircleCoord;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec4 uColor;
uniform vec2 uTiling;
uniform int uIsCircle;
uniform int uHasTexture;

void main() {
    vec4 baseColor = uColor;

    if(uHasTexture == 1) {
        vec4 tex = texture(uTexture, TexCoord * uTiling);
       baseColor *= tex;
    }

    if(uIsCircle == 1) {
        if(dot(CircleCoord, CircleCoord) > 1.0){
            discard;
        }
    }

    FragColor = baseColor;
}