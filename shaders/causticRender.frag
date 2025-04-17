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
    color = vec4(1, 1, 1, 1);
}
