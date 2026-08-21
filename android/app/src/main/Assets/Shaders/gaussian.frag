#version 300 es
precision mediump float;
layout (location=0) out vec4 frag_color;
  
in vec2 vTexCoord;

uniform sampler2D uTex2d;
  
uniform bool horizontal;
float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{             
    vec2 tex_offset = 1.0 / vec2(textureSize(uTex2d, 0)); // gets size of single texel
    vec3 result = texture(uTex2d, vTexCoord).rgb * weight[0]; // current fragment's contribution
    
    // horizontal blur pass
    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(uTex2d, vTexCoord + vec2(tex_offset.x * float(i), 0.0)).rgb * weight[i];
            result += texture(uTex2d, vTexCoord - vec2(tex_offset.x * float(i), 0.0)).rgb * weight[i];
        }
    }
    else // vertical blur pass
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(uTex2d, vTexCoord + vec2(0.0, tex_offset.y * float(i))).rgb * weight[i];
            result += texture(uTex2d, vTexCoord - vec2(0.0, tex_offset.y * float(i))).rgb * weight[i];
        }
    }
    frag_color = vec4(result, 1.0);
}