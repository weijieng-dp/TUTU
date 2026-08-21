#version 300 es
precision mediump float;

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;

out vec2 vTexCoord;

void main() {
    gl_Position = vec4(pos.x * 2.f,pos.y * 2.f, 0.0, 1.0);
    vTexCoord = uv;
}