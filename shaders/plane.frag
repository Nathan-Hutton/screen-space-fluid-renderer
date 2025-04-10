#version 330 core

layout(location = 0) out vec4 color;

in vec2 texCoords;

uniform sampler2D depthTex;

float LinearizeDepth(vec2 uv)
{
    float zNear = 0.1;    // TODO: Replace by the zNear of your perspective projection
    float zFar  = 5.0; // TODO: Replace by the zFar  of your perspective projection
    float depth = texture2D(depthTex, uv).x;
    return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

void main()
{
	float depth = LinearizeDepth(texCoords);

    // float depth = texture(depthTex, texCoords).r;
    color = vec4(depth, depth, depth, 1);
    // color = vec4(texCoords.x, texCoords.y, 0, 1.0);
}
