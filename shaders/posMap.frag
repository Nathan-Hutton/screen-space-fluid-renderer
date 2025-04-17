#version 330 core

in vec3 ogPos;

layout(location = 0) out vec4 color;

void main()
{
    color = vec4(ogPos, 1.0);
}