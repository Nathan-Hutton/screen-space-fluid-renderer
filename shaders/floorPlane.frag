#version 330 core

// in vec3 Normal;
// in vec3 LightDir;
//in vec3 ViewDir;
in vec2 texCoord;

uniform sampler2D difTex;
// uniform sampler2D spcTex;

layout(location = 0) out vec4 color;

vec3 calcDiffuse(vec3 difColor, vec3 normal, vec3 lightDir)
{
	float difAmount = max(dot(normal, lightDir), 0);
	vec3 outColor =  difColor * difAmount;
	return outColor;
}

vec3 calcSpecular(vec3 specColor, vec3 normal, vec3 lightDir, vec3 viewDir, float shine)
{
	vec3 halfAngle = normalize(lightDir + viewDir);
	float cosPhi = max(dot(normal, halfAngle), 0);
	vec3 outColor = specColor * pow(cosPhi, shine);

	return outColor;
}

void main()
{
	// vec3 normal = normalize(Normal);
	// vec3 lightDir = normalize(LightDir);
	//vec3 viewDir = normalize(ViewDir);

	vec3 difColor = vec3(texture(difTex, texCoord));
	//vec3 specColor = vec3(texture( spcTex, texCoord ));
	//vec3 difColor = vec3(1, 1, 0);
	//vec3 specColor = vec3(1, 1, 1);
	//float shine = 20;

	//vec3 ambient = 0.2 * difColor;
	// vec3 diffuse = calcDiffuse(difColor, normal, lightDir);
	//vec3 specular = calcSpecular(specColor, normal, lightDir, viewDir, shine);

	// vec3 total = clamp(ambient + diffuse + specular, 0, 1);
	color = vec4(difColor, 1);
}