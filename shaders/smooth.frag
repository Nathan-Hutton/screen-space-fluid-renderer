#version 330 core

layout(location = 0) out float smooth_depth;

in vec2 texCoords;

uniform sampler2D depthTex;
uniform float e;    // mathematical constant
uniform float pi;   // mathematical constant

float filterRadius = 0.5;
float delta;
float mu;


// float LinearizeDepth(vec2 uv)
// {
//     float zNear = 0.1;    // TODO: Replace by the zNear of your perspective projection
//     float zFar  = 5.0; // TODO: Replace by the zFar  of your perspective projection
//     float depth = texelFetch(depthTex, ivec2(uv), 0).x;
//     // float depth = texture2D(depthTex, uv).x;
//     return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
// }

void main()
{
    if ( texelFetch(depthTex, ivec2(gl_FragCoord.xy), 0).r >= 1.0 ) {
        gl_FragDepth = 1.0;
        return;
    }

    // let's try a basic gaussian filter
    float sigma = 10.0;
    float total = 0.0;
    float wgtNorm = 0.0;
    for (float i = 0.0; i < sigma * 3.0; i++) {
        float depth1 = texelFetch(depthTex, ivec2(gl_FragCoord.xy) + ivec2(i, 0), 0).r;
        float depth2 = texelFetch(depthTex, ivec2(gl_FragCoord.xy) + ivec2(-i, 0), 0).r;

        float expon = -(pow(i, 2.0) / (2.0 * pow(sigma, 2)));
        float denom = sqrt(2.0 * pi) * sigma;
        float weight = pow(e, expon) / denom;

        if (i == 0) {   // this is the center, and should only be counted once
            total += weight * depth1;
            wgtNorm += weight;
        }
        else {
            total += weight * (depth1 + depth2);
            wgtNorm += 2 * weight;
        }
    }
    total /= wgtNorm;
    // gl_FragDepth = texelFetch(depthTex, ivec2(gl_FragCoord.xy), 0).r;
    gl_FragDepth = total;
}