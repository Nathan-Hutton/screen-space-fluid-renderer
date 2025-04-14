#version 330 core

layout(location = 0) out vec4 color;

in vec2 texCoords;
// in vec3 lightPos;

uniform sampler2D depthTex;
uniform int imgW;
uniform int imgH;
uniform float scale;
uniform vec4 lightView;

vec3 camPos = vec3(imgW/2.0, imgH/2.0, -1.0);
// vec4 lightPosWorld = view * vec4(-3.0, 8.0, 7.0, 1.0);

float LinearizeDepth(vec2 uv)
{
    float zNear = 0.1;    // TODO: Replace by the zNear of your perspective projection
    float zFar  = 5.0; // TODO: Replace by the zFar  of your perspective projection
    float depth = texelFetch(depthTex, ivec2(uv), 0).x;
    // float depth = texture2D(depthTex, uv).x;
    return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

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
    float depth = LinearizeDepth(gl_FragCoord.xy);

    float w_float = float(imgW);
    float h_float = float(imgH);
    float dx = depth * scale;
    float dy = dx * h_float / w_float;

    dx = dx / w_float;
    dy = dy / h_float;

    float dzx = LinearizeDepth(ivec2(gl_FragCoord.xy) + ivec2(1, 0)) - depth;
    float dzy = LinearizeDepth(ivec2(gl_FragCoord.xy) + ivec2(0, 1)) - depth;

    // if (dzx == 0 || dzy == 0) {
    if ( texelFetch(depthTex, ivec2(gl_FragCoord.xy), 0).r >= 1.0 ) {
        color = vec4(0, 0, 0, 0);
        return;
    }

    vec3 normal = normalize(cross(vec3(dx, 0.0, -dzx), vec3(0.0, dy, -dzy)));

    // now we need to actually do some shading with this normal value
    // get light position in screen space
    vec4 lightNorm = lightView / abs(lightView.w);
    vec3 lightPos = vec3((lightNorm.x + 1) / 2 * imgW, (lightNorm.y + 1) / 2 * imgH, lightView.z);

    // fragment position with depth included
    vec3 fragPos = vec3(gl_FragCoord.xy, depth);

    // light and view directions for this fragment
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 viewDir = normalize(camPos - fragPos);

    //------ when it comes to the above light and view calculations, i suspect i did something wrong -----//

    // material parameters
    vec3 difColor = vec3(0.0, 1.0, 1.0);
    vec3 specColor = vec3(1.0, 1.0, 1.0);
    float shine = 10.0;

    vec3 diffuse = 0.5 * calcDiffuse(difColor, normal, lightDir);
    vec3 spec = calcSpecular(specColor, normal, lightDir, viewDir, shine);
    vec3 amb = 0.2 * difColor;

    vec3 total = clamp(diffuse + spec + amb, 0, 1);
    float alpha = clamp(spec.x * 2, 0.5, 1); // this is to make specular areas more opaque

    color = vec4(total, alpha);
    // color = vec4(normal, 1.0);
}
