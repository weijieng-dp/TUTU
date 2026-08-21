#version 300 es
precision mediump float;

in vec2 vTexCoord;

uniform vec4        uColour;
uniform sampler2D   uCurrColorTex;
uniform sampler2D   uCurrIntensityTex;
uniform float       uIntensity;
uniform vec2        uScreenResolution;
uniform bool        uFirstPass;

layout (location=0) out vec4 frag_color;
layout (location=1) out float frag_intensity;

// this shader is unused
void main() {
    frag_color = uColour;
    frag_intensity = uIntensity;
    // if(!uFirstPass)
    // {
        vec2 screenUV=  (gl_FragCoord.xy)/uScreenResolution.xy;
        float currIntensity = texture(uCurrIntensityTex,screenUV).r;

        float src = 0.0;
        if(currIntensity > 0.0f)
            src =(currIntensity)/(currIntensity + frag_intensity);
        src = clamp(src, 0.0f,1.0f);
        
        frag_intensity += currIntensity;
        frag_color.rgb = texture(uCurrColorTex,screenUV).rgb *  src + frag_color.rgb * (1.0 - src);
    // }

    //test
    // frag_color = vec4(screenUV,0.0,1.0);
}