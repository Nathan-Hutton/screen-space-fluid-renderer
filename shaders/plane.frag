#version 330 core

layout(location = 0) out vec4 color;

in vec2 texCoords;

uniform sampler2D depthTex;
uniform int imgW;
uniform int imgH;
uniform float scale;

float LinearizeDepth(vec2 uv)
{
    float zNear = 0.1;    // TODO: Replace by the zNear of your perspective projection
    float zFar  = 5.0; // TODO: Replace by the zFar  of your perspective projection
    float depth = texelFetch(depthTex, ivec2(gl_FragCoord.xy), 0).x;
    // float depth = texture2D(depthTex, uv).x;
    return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

void main()
{
    float depth = texelFetch(depthTex, ivec2(gl_FragCoord.xy), 0).r;

    float dx = depth * scale;
    float dy = dx * imgH / imgW;

    dx = dx / imgW;
    dy = dy / imgH;

    float dzx = texelFetch(depthTex, ivec2(gl_FragCoord.xy) + ivec2(1, 0), 0).r - depth;
    float dzy = texelFetch(depthTex, ivec2(gl_FragCoord.xy) + ivec2(0, 1), 0).r - depth;

    vec3 normal = normalize(cross(vec3(dx, 0, -dzx), vec3(0, dy, -dzy)));

    color = vec4(normal, 1);
}
