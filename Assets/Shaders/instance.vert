#version 300 es
precision mediump float;

// --- Per vertex data ----
layout(location = 0) in vec2 vp;
layout(location = 1) in vec2 aVertexTexCoord;

// --- Per instance data ----
layout(location = 2) in vec3 xformCol0;
layout(location = 3) in vec3 xformCol1;
layout(location = 4) in vec3 xformCol2;         // xformCol2.z holds the depth value
layout(location = 5) in vec2 spriteOrigin;
layout(location = 6) in vec2 spriteSize;
layout(location = 7) in uint aEntityID;
layout(location = 8) in vec4 color;
layout(location = 9) in vec2 tile;
layout(location = 10) in int isEmissive;

out vec4 outColor; 
out vec2 vTexCoord;
flat out uint vEntityID;
flat out vec2 vSpriteOrigin;
flat out vec2 vSpriteSize;
flat out vec2 vTile;
flat out int vIsEmissive;

uniform vec4 uDebugColor;
uniform mat3 uViewProj;
uniform bool uUseDebugColor;

void main() {
    float ndcDepth = xformCol2.z;
    
    mat3 instanceMatrix = mat3(xformCol0, xformCol1, vec3(xformCol2.xy, 1.0));
    vec3 pos = uViewProj * instanceMatrix * vec3(vp, 1.0);
    gl_Position = vec4(pos.xy, ndcDepth, 1.0);

    vTexCoord.x = aVertexTexCoord.x;
    vTexCoord.y = aVertexTexCoord.y;

    outColor = uUseDebugColor ? uDebugColor : color;
    vEntityID = aEntityID;
    vSpriteOrigin = spriteOrigin;
    vSpriteSize = spriteSize;
    vTile = tile;
    vIsEmissive = isEmissive;
}