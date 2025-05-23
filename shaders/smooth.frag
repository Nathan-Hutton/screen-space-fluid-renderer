#version 460

layout(location = 0) out vec4 color;

uniform sampler2D depthTex;
uniform bool horizontal; // Horizontal or vertical pass
uniform int verticalResolution;
uniform float verticalFOV;

// float narrowRangeThreshold = 0.0625f; // This is delta in the paper
float particle_r = 0.00325f;
float mu = particle_r;
float narrowRangeThreshold = particle_r * 10;
float near = 0.1;
float far = 5.0;

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

float gaussian_weight(float x, float two_sigma2)
{
    return exp(- (x * x) / two_sigma2);
}

void main()
{
    const float centerDepth = texelFetch(depthTex, ivec2(gl_FragCoord.xy), 0).r;
    if ( centerDepth >= 1.0 ) {
        gl_FragDepth = 1.0;
        color = vec4(1.0);
        return;
    }

    // Compute standard deviation
    // const float worldSigma = 0.1;
    const float worldSigma = 0.7 * particle_r; // from the paper
    const float viewDistance = getCameraSpaceDepthValue(centerDepth);
    float sigma = getAdjustableStandardDeviation(worldSigma, viewDistance);
    sigma = clamp(sigma, 0.5, 15.0);
    const float two_sigma2 = 2.0 * sigma * sigma;

    const ivec2 center = ivec2(gl_FragCoord.xy);
    const ivec2 offset = horizontal ? ivec2(1, 0) : ivec2(0, 1);

    float smoothedDepth = 0.0;
    float weightNorm = 0.0;

    const int radius = int(ceil(3.0 * sigma));
    for (int i = -radius; i <= radius; ++i)
    {
        const ivec2 samplePos = center + i * offset;
        const float depth = texelFetch(depthTex, samplePos, 0).r;
        // if (depth >= 1.0) continue;
        const float weight = (depth < (centerDepth - narrowRangeThreshold)) ? 0.0f : gaussian_weight(float(i), two_sigma2); // Eliminate contributions that are too close to the camera
        const float alteredDepth = (depth <= (centerDepth + narrowRangeThreshold)) ? depth : centerDepth + mu; // Limit depth values that are too far behind the center pixel

        smoothedDepth += weight * alteredDepth;
        weightNorm += weight;
    }

    if (weightNorm > 0.0)
        smoothedDepth /= weightNorm;
    else
        smoothedDepth = centerDepth;

    gl_FragDepth = smoothedDepth;
    color = vec4(smoothedDepth, smoothedDepth, smoothedDepth, 1.0);
}
