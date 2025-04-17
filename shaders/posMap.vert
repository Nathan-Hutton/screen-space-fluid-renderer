#version 330 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 norm;
layout(location=2) in vec2 txc;

uniform mat4 lvp;

out vec3 ogPos;

void main()
{
	gl_Position = lvp * vec4( pos, 1 );

    ogPos = pos;
}