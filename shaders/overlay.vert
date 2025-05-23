#version 330 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 norm;
layout(location=2) in vec2 txc;

uniform mat4 mvp;

out vec2 texCoord;
out vec3 ogPos;

void main()
{
	gl_Position = mvp * vec4( pos, 1 );

    texCoord = txc;
    ogPos = pos;
}