#version 300 es
precision mediump float;

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;

uniform float       ndcDepth;
uniform mat3        mdl_to_ndc;

out vec2 vTexCoord;

void main() {
    vec3 pos = mdl_to_ndc * vec3(pos, 1.0);
    gl_Position = vec4(pos.xy, ndcDepth, 1.0);

    vTexCoord = vec2(uv.x, 1.f - uv.y);
}