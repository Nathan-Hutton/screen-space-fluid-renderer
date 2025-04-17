#version 460 core

layout(location = 0) out vec4 color;

in vec2 texCoords;

uniform sampler2D posMap;
uniform mat4 invProjectionMatrix;
uniform mat4 invViewMatrix;
uniform mat4 lvp;
uniform sampler2D depthTex;
uniform int imgW;
uniform int imgH;

float zNear = 0.1;
float zFar  = 15.0;
float eta = 1.0/1.33;

float LinearizeDepth(vec2 uv)
{
    float depth = texture(depthTex, uv).x;
    return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

vec3 uvToEye(vec2 texCoord, float depth)
{
    float x  = texCoord.x * 2.0 - 1.0;
    float y  = texCoord.y * 2.0 - 1.0;
    float zn = LinearizeDepth(texCoord) * 2.0 - 1.0;

    vec4 clipPos = vec4(x, y, zn, 1.0f);
    vec4 viewPos = invProjectionMatrix * clipPos;
    return viewPos.xyz / viewPos.w;
}

vec4 uvToWorld(vec2 texCoord, float eyeDepth)
{
    float x  = texCoord.x * 2.0 - 1.0;
    float y  = texCoord.y * 2.0 - 1.0;
    float zn = LinearizeDepth(texCoord) * 2.0 - 1.0;

    vec4 clipPos = vec4(x, y, zn, 1.0f);
    vec4 viewPos = invProjectionMatrix * clipPos;
    vec4 eyePos  = viewPos.xyzw / viewPos.w;

    vec4 worldPos = invViewMatrix * eyePos;
    return worldPos;
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

    color = vec4(normal, 1.0);
}