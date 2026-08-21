#version 300 es
precision mediump float;

in vec2 vTexCoord;

uniform sampler2D sceneTex;
uniform sampler2D lightColorTex;
uniform sampler2D lightIntensityTex;


layout (location=0) out vec4 frag_color;

void main() {
    // blend scene color scene * light.combinedColor * combinedIntensity
    frag_color = texture(sceneTex, vTexCoord) * vec4(texture(lightColorTex, vTexCoord).rgb,1.0) * texture(lightIntensityTex, vTexCoord).r;
}