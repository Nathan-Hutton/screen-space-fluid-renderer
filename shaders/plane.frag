#version 460 core

layout(location = 0) out vec4 color;

in vec2 texCoords;
// in vec3 lightPos;

uniform samplerCube env;
// uniform mat4 projectionMatrix;
uniform mat4 invProjectionMatrix;
uniform mat4 invViewMatrix;
uniform sampler2D depthTex;
uniform int imgW;
uniform int imgH;
// uniform float scale;
// uniform vec4 lightView;

vec3 lightPosWorld = vec3(-3.0, 8.0, 7.0);
vec3 camPosWorld = vec3(0.93, 0.47, -1.51);
// vec4 lightPosWorld = view * vec4(-3.0, 8.0, 7.0, 1.0);
float zNear = 0.1;
float zFar  = 5.0;
float eta = 1.0/1.33;
float fresnelPower = 5.0;
float F = ((1.0 - eta) * (1.0 - eta)) / ((1.0 + eta) * (1.0 + eta));

float LinearizeDepth(vec2 uv)
{
    float depth = texelFetch(depthTex, ivec2(uv), 0).x;
    // float depth = texture2D(depthTex, uv).x;
    return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

vec3 uvToEye(vec2 texCoord, float depth)
{
    float x  = texCoord.x * 2.0 - 1.0;
    float y  = texCoord.y * 2.0 - 1.0;
    float zn = ((zFar + zNear) / (zFar - zNear) * depth + 2 * zFar * zNear / (zFar - zNear)) / depth;

    vec4 clipPos = vec4(x, y, zn, 1.0f);
    vec4 viewPos = invProjectionMatrix * clipPos;
    return viewPos.xyz / viewPos.w;
}

vec4 uvToWorld(vec2 texCoord, float eyeDepth)
{
    float x  = texCoord.x * 2.0 - 1.0;
    float y  = texCoord.y * 2.0 - 1.0;
    float zn = ((zFar + zNear) / (zFar - zNear) * eyeDepth + 2 * zFar * zNear / (zFar - zNear)) / eyeDepth;

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
    if ( eyeDepth >= 1.0 ) {
        // color = vec4(1, 0, 0, 0);
        discard;    // uhhhh this might not be best practice, i'll look that up later
        return;
    }
    // color = vec4(eyeDepth, eyeDepth, eyeDepth, 1.0);
    // return;


    float pixelWidth  = 1 / float(imgW);
    float pixelHeight = 1 / float(imgW);
    
    float xDepth = texture(depthTex, texCoords + vec2(pixelWidth, 0)).x;
    float yDepth = texture(depthTex, texCoords + vec2(0, pixelHeight)).x;

    vec4 worldPos = uvToWorld(texCoords, eyeDepth);
    vec4 dxWorld = uvToWorld(texCoords + vec2(pixelWidth, 0), xDepth);
    vec4 dyWorld = uvToWorld(texCoords + vec2(0, pixelHeight), yDepth);

    vec3 dzx = normalize(vec3(dxWorld - worldPos));
    vec3 dzy = normalize(vec3(dyWorld - worldPos));

    vec3 normal = normalize(cross(dzx, dzy));

    // now we need to actually do some shading with this normal value
    // get light position in screen space
    // vec4 lightNorm = lightView / abs(lightView.w);
    vec3 lightDir = normalize(lightPosWorld - vec3(worldPos));

    // // fragment position with depth included
    // vec3 fragPos = vec3(gl_FragCoord.xy, depth);

    // // light and view directions for this fragment
    // vec3 lightDir = normalize(lightPos - fragPos);
    vec3 viewDir = normalize(camPosWorld - vec3(worldPos));

    // //------ when it comes to the above light and view calculations, i suspect i did something wrong -----//

    // // material parameters
    // vec3 difColor = vec3(0.0, 1.0, 1.0);
    float fresnelRatio    = clamp(F + (1.0 - F) * pow((1.0 - dot(-viewDir, normal)), fresnelPower), 0, 1);
    vec3 reflDir = reflect( viewDir, normal );
    vec3 refrDir = refract( viewDir, normal, eta);
    vec3 reflColor = vec3(texture( env, refrDir ));
    vec3 refrColor = vec3(texture( env, refrDir ));
    vec3 diffuseColor = mix(refrColor, reflColor, fresnelRatio);

    vec3 specColor = vec3(1.0, 1.0, 1.0);
    float shine = 10.0;

    // vec3 diffuse = calcDiffuse(difColor, normal, lightDir);
    vec3 spec = calcSpecular(specColor, normal, lightDir, viewDir, shine);
    // vec3 amb = 0.2 * difColor;

    vec3 total = clamp(diffuseColor + spec, 0, 1);
    // float alpha = clamp(spec.x * 2, 0.5, 1); // this is to make specular areas more opaque

    color = vec4(total, 1.0);
    // color = vec4(reflDir, 1.0);
    // color = vec4(dzy, 1.0);
}
