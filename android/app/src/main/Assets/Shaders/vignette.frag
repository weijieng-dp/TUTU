#version 300 es
precision mediump float;

in vec2 vTexCoord;

uniform float uIntensity;
uniform float uFalloff;

layout (location=0) out vec4 frag_color;

void main() {
	vec2 uv = vTexCoord;
    uv = (uv - 0.5) * 2.f;
    
    float vig = length(uv);
    vig = pow(vig, uFalloff);
    vig *= uIntensity;
    // vig = pow(vig, 0.5); // change pow for modifying the extend of the  vignette
    frag_color = vec4(0.0,0.0,0.0,vig);
    // frag_color = vec4(uv.x,uv.y,0.0,1.0);
}