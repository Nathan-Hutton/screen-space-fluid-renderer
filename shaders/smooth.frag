#version 330 core

layout(location = 0) out vec4 color;

in vec2 texCoords;

uniform sampler2D depthTex;

float compute_weight2D(vec2 r, float two_sigma2)
{
    return exp(-dot(r, r) / two_sigma2);
}

void main()
{
    float depth = texelFetch(depthTex, ivec2(gl_FragCoord.xy), 0).r;
    if ( depth >= 1.0 ) {
        gl_FragDepth = 1.0;
        color = vec4(1.0);
        return;
    }

    // let's try a basic gaussian filter
    const float sigma = 10.0;
    const float two_sigma2 = 2.0 * sigma * sigma;
    
    float total = depth;
    float wgtNorm = 1.0;

    const int radius = int(sigma) * 3;
    for (int i = -radius; i <= radius; ++i) {
        for (int j = -radius; j <= radius; ++j) {
            float depth = texelFetch(depthTex, ivec2(gl_FragCoord.xy) + ivec2(i, j), 0).r;
            float weight = compute_weight2D(vec2(i, j), two_sigma2);
            total += weight * depth;
            wgtNorm += weight;
        }
    }

    total /= wgtNorm;
    gl_FragDepth = total;
    color = vec4(total, total, total, 1.0);
}
