#version 300 es
precision mediump float;

in vec4 outColor;
in vec2 vTexCoord;
flat in uint vEntityID;
flat in vec2 vSpriteOrigin;
flat in vec2 vSpriteSize;
flat in vec2 vTile;
flat in int vIsEmissive;

uniform sampler2D   uTex2d;
uniform sampler2D   uEmmissionTex2d;
uniform bool        uUseTex;

layout (location=0) out vec4 frag_color;
layout (location=1) out uint entity_id;
layout (location=2) out vec4 bright_color;

void main() {
    if(uUseTex)
    {
        vec2 uv;
        uv.x = vSpriteOrigin.x + vSpriteSize.x * fract(vTexCoord.x * vTile.x);
        uv.y = vSpriteOrigin.y + vSpriteSize.y * fract(vTexCoord.y * vTile.y);
        frag_color = texture(uTex2d, uv) * outColor;

        switch(vIsEmissive)
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
        uv.x = vSpriteOrigin.x + vSpriteSize.x * fract(vTexCoord.x * vTile.x);
        uv.y = vSpriteOrigin.y + vSpriteSize.y * fract(vTexCoord.y * vTile.y);        
        frag_color = outColor;
        switch(vIsEmissive)
        {
            case 1:
                bright_color = outColor; 
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

    entity_id = vEntityID;
}