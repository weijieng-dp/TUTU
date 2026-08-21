#version 300 es
precision mediump float;
layout (location=0) out vec4 frag_color;
  
in vec2 vTexCoord;

uniform sampler2D sceneTex;
uniform sampler2D blurTex;
uniform float exposure;
// uniform float gamma;

void main()
{             
    // scene color
    vec3 hdrColor = texture(sceneTex, vTexCoord).rgb;      
    // blur color
    vec3 bloomColor = texture(blurTex, vTexCoord).rgb;
    
    // get luminosity
    float brightness = dot(hdrColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    
    // blend blur with scene color, inversly proportional to the brightness
    hdrColor += bloomColor * (1.0 - pow(brightness, exposure)); // additive blending
    // hdrColor = vec3(1.0) - exp(-hdrColor * exposure);
    // hdrColor = pow(hdrColor, vec3(1.0 / gamma));
    frag_color = vec4(hdrColor, 1.0);
    // frag_color = vec4(1.0,0.0,1.0, 1.0);    
}  