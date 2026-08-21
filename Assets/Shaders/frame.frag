#version 300 es
precision mediump float;

in vec2 vTexCoord;

uniform sampler2D   uTex2d;

layout (location=0) out vec4 frag_color;

void main() {
	vec2 uv = vTexCoord;
   
    uv *=  1.0 - uv.yx;
    frag_color = texture(uTex2d, vTexCoord); //* vig;
}