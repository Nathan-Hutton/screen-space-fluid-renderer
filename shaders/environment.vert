#version 460 core

layout (location = 0) in vec3 aPos;

out vec3 dir;

uniform mat4 invViewProjection;

void main()
{
    dir = vec3(invViewProjection * vec4(aPos, 1.0));
    gl_Position = vec4(aPos, 1.0);
}
