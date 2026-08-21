#version 300 es
precision mediump float;

layout(location = 0) in vec2 vp;
layout(location = 1) in vec2 aVertexTexCoord;

uniform float       ndcDepth;
uniform mat3        mdl_to_ndc;

out vec2 vTexCoord;

void main() {
    vec3 pos = mdl_to_ndc * vec3(vp, 1.0);
    gl_Position = vec4(pos.xy, ndcDepth, 1.0);
    
    vTexCoord.x = aVertexTexCoord.x;
    vTexCoord.y = aVertexTexCoord.y;
}