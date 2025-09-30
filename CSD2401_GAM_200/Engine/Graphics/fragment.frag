#version 450 core
in vec2 TexCoord;
in vec2 CircleCoord;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec4 uColor;
uniform vec2 uTiling;
uniform vec2 uTexOffset;
uniform int uIsCircle;
uniform int uHasTexture;

void main() {
    vec4 baseColor = uColor;

   if(uHasTexture == 1) {
      vec2 finalTexCoord = TexCoord * uTiling + uTexOffset; 
        
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