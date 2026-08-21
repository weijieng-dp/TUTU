#version 300 es
precision mediump float;

in vec2 vTexCoord;

uniform sampler2D   uTex2d;
uniform vec4        uColour;
uniform uint        uEntityID;

layout (location=0) out vec4 frag_color;
layout (location=1) out uint entity_id;

void main() {
    frag_color = vec4(uColour.rgb, texture(uTex2d, vTexCoord).r * uColour.a);   
    if (frag_color.a < 0.01f)
	discard;
    entity_id =  uEntityID;
}