#version 330 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 norm;
layout(location=2) in vec2 txc;

uniform mat4 mvp;
// uniform mat4 mShadow;
uniform mat4 lvp;

out vec2 texCoord;
out vec4 shadowPos;

void main()
{
	gl_Position = mvp * vec4( pos, 1.0 );
    shadowPos = lvp * vec4( pos, 1.0 );

    texCoord = txc;
}