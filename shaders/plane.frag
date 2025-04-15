#version 460 core

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
    float depth = texelFetch(depthTex, ivec2(uv), 0).x;
    // float depth = texture2D(depthTex, uv).x;
    return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

void main()
{
    // float depth = texelFetch(depthTex, ivec2(gl_FragCoord.xy), 0).r;
    float depth = LinearizeDepth(gl_FragCoord.xy);

    float w_float = float(imgW);
    float h_float = float(imgH);
    float dx = depth * scale;
    float dy = dx * h_float / w_float;

    dx = dx / w_float;
    dy = dy / h_float;

    // float dzx = texelFetch(depthTex, ivec2(gl_FragCoord.xy) + ivec2(1, 0), 0).r - depth;
    // float dzy = texelFetch(depthTex, ivec2(gl_FragCoord.xy) + ivec2(0, 1), 0).r - depth;

    float dzx = LinearizeDepth(ivec2(gl_FragCoord.xy) + ivec2(1, 0)) - depth;
    float dzy = LinearizeDepth(ivec2(gl_FragCoord.xy) + ivec2(0, 1)) - depth;

    // if (dzx == 0 || dzy == 0) {
    if ( texelFetch(depthTex, ivec2(gl_FragCoord.xy), 0).r >= 1.0 ) {
        color = vec4(0, 0, 0, 0);
        return;
    }

    vec3 normal = normalize(cross(vec3(dx, 0.0, -dzx), vec3(0.0, dy, -dzy)));
    // vec3 normal = normalize(vec3(dzx/dx, dzy/dy, 1.0));

    color = vec4(normal, 1.0);
    // color = vec4(depth, depth, depth, 1);
    // color = texelFetch(depthTex, ivec2(gl_FragCoord.xy), 0);
}
