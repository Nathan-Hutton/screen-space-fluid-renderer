#version 460

layout(location = 0) out vec4 color;

uniform sampler2D depthTex;
uniform bool horizontal; // Horizontal or vertical pass
uniform float near;
uniform float far;
uniform int verticalResolution;
uniform float verticalFOV;

// We need unnormalized camera depth values
float getCameraSpaceDepthValue(float normalizedDepth)
{
    float value = normalizedDepth * 2.0 - 1.0; // Bring values to [-1, 1] range
    return (2.0 * near * far) / (far + near - value * (far - near));
}

float getAdjustableStandardDeviation(float worldSpaceSmooth, float cameraDepth)
{
    return (verticalResolution * worldSpaceSmooth) / (2.0 * abs(cameraDepth) * tan(verticalFOV * 0.5));
}

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

    // Compute standard deviation
    const float worldSigma = 0.2;
    float viewDistance = getCameraSpaceDepthValue(centerDepth);
    float sigma = getAdjustableStandardDeviation(worldSigma, viewDistance);
    sigma = clamp(sigma, 0.5, 15.0);
    const float two_sigma2 = 2.0 * sigma * sigma;

    ivec2 center = ivec2(gl_FragCoord.xy);
    ivec2 offset = horizontal ? ivec2(1, 0) : ivec2(0, 1);

    float smoothedDepth = 0.0;
    float weightNorm = 0.0;

    int radius = int(ceil(3.0 * sigma));
    for (int i = -radius; i <= radius; ++i)
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
