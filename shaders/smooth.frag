#version 460

layout(location = 0) out vec4 color;

uniform sampler2D depthTex;
uniform bool horizontal; // Horizontal or vertical pass

float compute_weight1D(float x, float two_sigma2)
{
    return exp(- (x * x) / two_sigma2);
}

void main()
{
    // let's try a basic gaussian filter
    float centerDepth = texelFetch(depthTex, ivec2(gl_FragCoord.xy), 0).r;
    if ( centerDepth >= 1.0 ) {
        gl_FragDepth = 1.0;
        color = vec4(1.0);
        return;
    }

    float minSigma = 3.0;
    float maxSigma = 20.0;

    float sigma = mix(minSigma, maxSigma, centerDepth);
    //const float sigma = 10.0;
    const float two_sigma2 = 2.0 * sigma * sigma;

    ivec2 center = ivec2(gl_FragCoord.xy);
    ivec2 offset = horizontal ? ivec2(1, 0) : ivec2(0, 1);

    float smoothedDepth = 0.0;
    float weightNorm = 0.0;

    for (int i = -32; i <= 32; ++i)
    {
        ivec2 samplePos = center + i * offset;
        float depth = texelFetch(depthTex, samplePos, 0).r;
        if (depth < 1.0)
        {
            float weight = compute_weight1D(float(i), two_sigma2);
            smoothedDepth += weight * depth;
            weightNorm += weight;
        }
    }

    smoothedDepth /= weightNorm;
    gl_FragDepth = smoothedDepth;
    color = vec4(smoothedDepth, smoothedDepth, smoothedDepth, 1.0);
}
