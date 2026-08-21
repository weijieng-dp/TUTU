#version 300 es
precision mediump float;

in vec2 vTexCoord;

uniform sampler2D   uSceneTex;
uniform sampler2D   uEmissiveTex;
uniform float       uThreshold;

layout (location=0) out vec4 frag_color;

void main() {
    vec3 emissiveColor = texture(uEmissiveTex, vTexCoord).rgb;
    vec3 sceneColor = texture(uSceneTex, vTexCoord).rgb;
    // if there is an emissive color
    if(any(notEqual(emissiveColor, vec3(0,0,0))))
    {
        frag_color = vec4(sceneColor.rgb, 1.0);
        return;
    }
    

    float brightness = dot(sceneColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    
    // write "bright" pixels to the buffer
    if(brightness > uThreshold)
        frag_color = vec4(sceneColor.rgb, 1.0);
    else
        frag_color = vec4(0.0, 0.0, 0.0,0.0);
}