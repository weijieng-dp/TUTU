#version 300 es
precision mediump float;

in vec2 vTexCoord;

uniform bool        uUseTex;
uniform sampler2D   uTex2d;
uniform int         uEmissionState;
uniform sampler2D   uEmmissionTex2d;
uniform vec4        uColour;
uniform uint        uEntityID;
uniform vec2        tile;
uniform vec2        spriteOrigin;
uniform vec2        spriteSize;

layout (location=0) out vec4 frag_color;
layout (location=1) out uint entity_id;
layout (location=2) out vec4 bright_color;

void main() {
    bright_color = vec4(0.0,0.0,0.0,0.0);
    if(uUseTex)
    {
        vec2 uv;
        uv.x = spriteOrigin.x + spriteSize.x * fract(vTexCoord.x * tile.x);
        uv.y = spriteOrigin.y + spriteSize.y * fract(vTexCoord.y * tile.y);
        frag_color = texture(uTex2d, uv) * uColour;
        
        switch(uEmissionState)
        {
            case 1:
                bright_color = texture(uTex2d, uv);
            break;
            case 2:
                bright_color = texture(uEmmissionTex2d, uv);
                if(bright_color.a == 0.0)
                    bright_color = vec4(0.0,0.0,0.0,0.0);
            break;
            default:
                bright_color = vec4(0.0,0.0,0.0,0.0);
            break;
        }
    }
    else
    {
        vec2 uv;
        uv.x = spriteOrigin.x + spriteSize.x * fract(vTexCoord.x * tile.x);
        uv.y = spriteOrigin.y + spriteSize.y * fract(vTexCoord.y * tile.y);
        frag_color = uColour;
        switch(uEmissionState)
        {
            case 1:
            bright_color = uColour;
            break;
            case 2:
                bright_color = texture(uEmmissionTex2d, uv);
                if(bright_color.a == 0.0)
                    bright_color = vec4(0.0,0.0,0.0,0.0);
            break;
            default:
                bright_color = vec4(0.0,0.0,0.0,0.0);
            break;
        }
    }

    if(frag_color.a < 0.01)
        discard;

    entity_id =  uEntityID;
}