#version 460

layout(location = 0) out vec4 color;

in vec2 texCoords;

uniform sampler2D depthTex;
uniform bool horizontal; // Horizontal or vertical pass

const float gWeights32[32] ={
   0.02954, 0.03910, 0.03844, 0.03743, 0.03609, 0.03446, 0.03259, 0.03052, 
   0.02830, 0.02600, 0.02365, 0.02130, 0.01900, 0.01679, 0.01469, 0.01272, 
   0.01092, 0.00928, 0.00781, 0.00651, 0.00537, 0.00439, 0.00355, 0.00285, 
   0.00226, 0.00178, 0.00138, 0.00107, 0.00081, 0.00062, 0.00046, 0.00034
};
const float gOffsets32[32] ={
   0.66640, 2.49848, 4.49726, 6.49605, 8.49483, 10.49362, 12.49240, 14.49119,
   16.48997, 18.48876, 20.48754, 22.48633, 24.48511, 26.48390, 28.48268, 30.48147, 
   32.48026, 34.47904, 36.47783, 38.47662, 40.47540, 42.47419, 44.47298, 46.47176, 
   48.47055, 50.46934, 52.46813, 54.46692, 56.46571, 58.46450, 60.46329, 62.46208
};

void main()
{
    // let's try a basic gaussian filter
    float centerDepth = texture(depthTex, texCoords).r;
    if ( centerDepth >= 1.0 ) {
        gl_FragDepth = 1.0;
        color = vec4(1.0);
        return;
    }

    const vec2 texelSize = 1.0f / textureSize(depthTex, 0); // Get the size of a texel (unit of a texture map)
    const vec2 texOffset = horizontal ? vec2(texelSize.x, 0.0) : vec2(0.0, texelSize.y);

    float smoothedDepth = centerDepth * gWeights32[0];
    float weightNorm = gWeights32[0];
    for (int i = 0; i < 32; ++i)
    {
        float weight = gWeights32[i];
        float depth1 = texture(depthTex, texCoords + gOffsets32[i] * texOffset).r;
        float depth2 = texture(depthTex, texCoords - gOffsets32[i] * texOffset).r;

        smoothedDepth += weight * depth1;
        smoothedDepth += weight * depth2;
        weightNorm += weight * 2.0;
    }
    smoothedDepth /= weightNorm;
    gl_FragDepth = smoothedDepth;
    color = vec4(smoothedDepth, smoothedDepth, smoothedDepth, 1.0);
}
