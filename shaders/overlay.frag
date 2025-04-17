#version 330 core

in vec2 texCoord;
in vec3 ogPos;

// uniform sampler2D difTex;
uniform sampler2D causticMap;
uniform mat4 lvp;

layout(location = 0) out vec4 color;

void main()
{
	vec4 lvpPos = lvp * vec4(ogPos, 1.0);
	vec2 uv = (lvpPos.xy / lvpPos.w) / 2.0 + 0.5;
	vec3 causColor = texture2D(causticMap, uv).xyz * 0.2;

    if (causColor.x == 0.0) {
        discard;
        return;
    }

	color = vec4(1.0, 1.0, 1.0, causColor.x);
}