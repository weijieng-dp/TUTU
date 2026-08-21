#version 300 es
precision mediump float;

in vec2 vTexCoord;

uniform sampler2D   uColourTex;
uniform sampler2D   uIntensityTex;

layout (location=0) out vec4 frag_color;
layout (location=1) out float frag_intensity;

void main() {
    frag_color = texture(uColourTex, vTexCoord);
    frag_intensity = texture(uIntensityTex, vTexCoord).r;
}