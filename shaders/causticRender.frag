#version 330 core

layout(location = 0) out vec4 color;

in vec3 destColor;
in vec2 uv;
in vec3 testPos;
in float render;

uniform sampler2D normalMap;
uniform sampler2D posMap;

void main()
{
    if (render == 0.0) { discard; return; }
    // color = vec4(1, 1, 1, 1);
    // color = texelFetch(posMap, ivec2(gl_FragCoord.xy), 0);
    vec3 myColor = texelFetch(posMap, ivec2(gl_FragCoord.xy), 0).xyz;
    if (abs(myColor.x) > 1.0 || abs(myColor.y) > 1.0 || abs(myColor.z) > 1.0) {
        color = vec4(0, 1, 0, 1);
        return;
    }
    color = vec4(myColor, 1);
}
