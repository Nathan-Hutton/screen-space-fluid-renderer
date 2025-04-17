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
    float shadow = currentDepth < closestDepth  ? 1.0 : 0.7;

    return shadow;
} 

void main()
{
	vec4 difColor = texture(difTex, texCoord);
	// vec2 uv = (shadowPos.xy) / 2.0 + 0.5;
	// float d1 = shadowPos.z / shadowPos.w;
	// float d2 = texture2D(depthMap, uv).x;

	// vec4 myPos = shadowPos * vec4(0.5, 0.5, 0.5, 1.0);
	// myPos += vec4(0.5, 0.5, 0.5, 0.0);

	// float d1 = myPos.z / myPos.w;
	// float d2 = texture2D(depthMap, myPos.xy).x;

	// if ((d1 - d2) > 0.0) {
	// 	color = vec4(1, 1, 1, 1);
	// }
	// else {
	// 	color = vec4(0, 0, 0, 1);
	// }

	difColor *= ShadowCalculation(shadowPos);
	// difColor *= textureProj(depthMap, shadowPos);
	// color = vec4(difColor, 1);
	color = vec4(difColor);
}