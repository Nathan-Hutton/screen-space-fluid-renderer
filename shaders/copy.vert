#version 460

layout(location=0) in vec3 pos;
layout(location=1) in vec3 norm;
layout(location=2) in vec2 txc;

out vec2 texCoords;

void main()
{
	gl_Position = vec4 (pos, 1);
    texCoords = txc;
}