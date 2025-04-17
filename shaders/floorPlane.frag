#version 330 core

in vec2 texCoord;
in vec4 shadowPos;

uniform sampler2D difTex;
uniform sampler2D depthMap;
// uniform sampler2DShadow depthMap;

layout(location = 0) out vec4 color;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(depthMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth < closestDepth  ? 1.0 : 0.85;

    return shadow;
} 

void main()
{
	vec4 difColor = texture(difTex, texCoord);

	difColor *= ShadowCalculation(shadowPos);

	color = vec4(difColor);
}