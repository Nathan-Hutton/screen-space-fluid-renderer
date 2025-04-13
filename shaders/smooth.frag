#version 330 core

layout(location = 0) out vec4 color;

in vec2 texCoords;

uniform sampler2D depthTex;
uniform float e;    // mathematical constatn
uniform float pi;   // mathematical constant

void main()
{
    // let's try a basic gaussian filter
    float sigma = 5.0;
    float total = 0.0;
    // float wgtNorm = 0.0;
    for (float i = 0.0; i < sigma * 3.0; i++) {
        float depth1 = texelFetch(depthTex, ivec2(gl_FragCoord.xy) + ivec2(i, 0), 0).r;
        float depth2 = texelFetch(depthTex, ivec2(gl_FragCoord.xy) + ivec2(-i, 0), 0).r;

        float expon = -(pow(i, 2.0) / (2.0 * pow(sigma, 2)));
        float denom = sqrt(2.0 * pi) * sigma;
        float weight = pow(e, expon) / denom;

        if (i == 0) {   // this is the center, and should only be counted once
            total += weight * depth1;
            // wgtNorm += weight;
        }
        else {
            total += weight * (depth1 + depth2);
            // wgtNorm += 2 * weight;
        }
    }
    // total /= wgtNorm;
    // float depth = texelFetch(depthTex, ivec2(gl_FragCoord.xy), 0).r;
    
    // color = texelFetch(depthTex, ivec2(gl_FragCoord.xy), 0);
    color = vec4(total, total, total, 1);
}