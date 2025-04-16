#version 460 core

layout(location = 0) out vec4 color;

in vec2 texCoords;
// in vec3 lightPos;

uniform samplerCube env;
// uniform mat4 projectionMatrix;
uniform mat4 invProjectionMatrix;
uniform mat4 invViewMatrix;
uniform mat4 projMatrix;
uniform sampler2D depthTex;
uniform int imgW;
uniform int imgH;
// uniform float scale;
// uniform vec4 lightView;

vec3 lightPosWorld = vec3(-3.0, 8.0, 7.0);
vec3 camPosWorld = vec3(0.93, 0.47, -1.51);
float zNear = 0.1;
float zFar  = 5.0;
float eta = 1.0/1.33;
float fresnelPower = 5.0;
float F = ((1.0 - eta) * (1.0 - eta)) / ((1.0 + eta) * (1.0 + eta));

float LinearizeDepth(vec2 uv)
{
    // float depth = texelFetch(depthTex, ivec2(uv), 0).x;
    float depth = texture(depthTex, uv).x;
    return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

vec3 uvToEye(vec2 texCoord, float depth)
{
    float x  = texCoord.x * 2.0 - 1.0;
    float y  = texCoord.y * 2.0 - 1.0;
    // float zn = ((zFar + zNear) / (zFar - zNear) * depth + 2 * zFar * zNear / (zFar - zNear)) / depth;
    float zn = LinearizeDepth(texCoord) * 2.0 - 1.0;

    vec4 clipPos = vec4(x, y, zn, 1.0f);
    vec4 viewPos = invProjectionMatrix * clipPos;
    return viewPos.xyz / viewPos.w;
}

vec4 uvToWorld(vec2 texCoord, float eyeDepth)
{
    float x  = texCoord.x * 2.0 - 1.0;
    float y  = texCoord.y * 2.0 - 1.0;
    // float zn = ((zFar + zNear) / (zFar - zNear) * eyeDepth + 2 * zFar * zNear / (zFar - zNear)) / eyeDepth;
    float zn = LinearizeDepth(texCoord) * 2.0 - 1.0;

    vec4 clipPos = vec4(x, y, zn, 1.0f);
    vec4 viewPos = invProjectionMatrix * clipPos;
    vec4 eyePos  = viewPos.xyzw / viewPos.w;

    vec4 worldPos = invViewMatrix * eyePos;
    return worldPos;
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
    float eyeDepth = texture(depthTex, texCoords).x;

    if ( eyeDepth >= 1.0 ) {    // skip background fragments
        discard;
        return;
    }

    // normal calculations in world space
    float pixelWidth  = 1 / float(imgW);
    float pixelHeight = 1 / float(imgW);
    
    float xDepth = texture(depthTex, texCoords + vec2(pixelWidth, 0)).x;
    float yDepth = texture(depthTex, texCoords + vec2(0, pixelHeight)).x;

    vec4 worldPos = uvToWorld(texCoords, eyeDepth);
    vec4 dxWorld = uvToWorld(texCoords + vec2(pixelWidth, 0), xDepth);
    vec4 dyWorld = uvToWorld(texCoords + vec2(0, pixelHeight), yDepth);

    vec3 dzx = vec3(dxWorld - worldPos);
    vec3 dzy = vec3(dyWorld - worldPos);

    vec3 normal = normalize(cross(dzx, dzy));

    // now we need to actually do some shading with this normal value
    vec3 lightDir = normalize(lightPosWorld - vec3(worldPos));

    vec3 eyePos = uvToEye(texCoords, eyeDepth);
    vec3 viewDir = normalize(-eyePos);
    viewDir = vec3(invViewMatrix* vec4(viewDir, 0));

    // material parameters
    // starting with fresnel fun
    float fresnelRatio    = clamp(F + (1.0 - F) * pow((1.0 - dot(viewDir, normal)), fresnelPower), 0, 1);
    vec3 reflDir = reflect( -viewDir, normal );
    vec3 refrDir = refract( -viewDir, normal, eta);
    vec3 reflColor = vec3(texture( env, reflDir ));
    vec3 refrColor = vec3(texture( env, refrDir ));

    // combine refraction and reflection components based on fresnel ratio
    fresnelRatio = mix(fresnelRatio, 1.0, 0.2);
    vec3 difColor = mix(refrColor, reflColor, fresnelRatio);

    // make the water more blue. for fun
    vec3 waterColor = vec3(0, 0.5, 1.0) * 0.5;
    difColor = mix(difColor, waterColor, 0.1);

    // add the specular highlights
    vec3 specColor = vec3(1.0, 1.0, 1.0);
    float shine = 50.0;     // i chose a high gloss so it doesn't look as metallic
    vec3 spec = calcSpecular(specColor, normal, lightDir, viewDir, shine);
    vec3 total = clamp(difColor + spec, 0, 1);

    color = vec4(total, 1.0);
}
