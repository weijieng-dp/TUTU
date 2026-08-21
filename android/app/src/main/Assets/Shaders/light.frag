#version 300 es
precision mediump float;

in vec2 vTexCoord;

uniform sampler2D   uLightShapeTex;
uniform sampler2D   uCurrColorTex;
uniform sampler2D   uCurrIntensityTex;
uniform vec4        uColour;
uniform float       uIntensity;
uniform vec2        uScreenResolution;
uniform bool        uFirstPass;

layout (location=0) out vec4 frag_color;
layout (location=1) out float frag_intensity;

void main() {
    frag_color = texture(uLightShapeTex,vTexCoord);

    // mix light shape texture with color
    frag_color.rgb *= uColour.rgb; 
    frag_intensity = uIntensity * frag_color.a;
    //if(!uFirstPass)
    // {
        vec2 screenUV=  (gl_FragCoord.xy)/uScreenResolution.xy;
        float currIntensity = texture(uCurrIntensityTex,screenUV).r;

        // normalize intensity to range of 0-1
        float src = 0.0;
        if(currIntensity > 0.0f)
            src =(currIntensity)/(currIntensity + frag_intensity);
        src = clamp(src, 0.0f,1.0f);

        frag_intensity += currIntensity;
        // blend src * lightColor + (1-src) * currColor 
        frag_color.rgb = texture(uCurrColorTex,screenUV).rgb *  src + frag_color.rgb * (1.0 - src);
    // }
}