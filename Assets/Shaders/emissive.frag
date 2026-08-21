#version 300 es
precision mediump float;

in vec2 vTexCoord;

uniform sampler2D   uEmissiveTex;

layout (location=0) out vec4 frag_color;
layout (location=1) out float frag_intensity;

void main() {
    vec3 emissiveColor = texture(uEmissiveTex,vTexCoord).rgb;
    if(any(notEqual(emissiveColor, vec3(0,0,0))))
    {
        frag_color.rgb = emissiveColor;
        frag_intensity = 1.0f;
    }
    else
        discard;
}