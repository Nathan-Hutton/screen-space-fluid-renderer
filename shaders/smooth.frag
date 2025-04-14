#version 330 core

layout(location = 0) out vec4 color;

in vec2 texCoords;

uniform sampler2D depthTex;
uniform float e;    // mathematical constant
uniform float pi;   // mathematical constant

float filterRadius = 0.5;
float delta;
float mu;

float compute_weight2D(vec2 r, float two_sigma2)
{
    return exp(-dot(r, r) / two_sigma2);
}

void main()
{
    float depth = texelFetch(depthTex, ivec2(gl_FragCoord.xy), 0).r;
    if ( depth >= 1.0 ) {
        gl_FragDepth = 1.0;
        return;
    }

    // let's try a basic gaussian filter
    float sigma = 10.0;
    float two_sigma2 = 2.0 * sigma * sigma;
    
    float total = depth;
    float wgtNorm = 1.0;

    for (float i = 1.0; i < sigma * 3.0; i++) {
        for (float j = 1.0; j < sigma * 3.0; j++) {
            float depth1 = texelFetch(depthTex, ivec2(gl_FragCoord.xy) + ivec2(i, j), 0).r;
            float depth2 = texelFetch(depthTex, ivec2(gl_FragCoord.xy) + ivec2(-i, j), 0).r;
            float depth3 = texelFetch(depthTex, ivec2(gl_FragCoord.xy) + ivec2(i, -j), 0).r;
            float depth4 = texelFetch(depthTex, ivec2(gl_FragCoord.xy) + ivec2(-i, -j), 0).r;

            float weight = compute_weight2D(vec2(i, j), two_sigma2);


            total += weight * (depth1 + depth2 + depth3 + depth4);
            wgtNorm += weight * 4;
        }
        

        // float expon = -(pow(i, 2.0) / (2.0 * pow(sigma, 2)));
        // float denom = sqrt(2.0 * pi) * sigma;
        // float weight = pow(e, expon) / denom;

        // if (i == 0) {   // this is the center, and should only be counted once
        //     total += weight * depth1;
        //     wgtNorm += weight;
        // }
        // else {
        //     total += weight * (depth1 + depth2);
        //     wgtNorm += 2 * weight;
        // }
    }
    total /= wgtNorm;
    // gl_FragDepth = texelFetch(depthTex, ivec2(gl_FragCoord.xy), 0).r;
    gl_FragDepth = total;
    color = vec4(total, total, total, 1.0);
}