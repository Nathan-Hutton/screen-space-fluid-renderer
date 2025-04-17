#version 330 core

layout(location=0) in vec3 pos;
// layout(location=1) in vec3 norm;
// layout(location=2) in vec2 txc;

uniform mat4 translate;
uniform mat4 lvp;
uniform mat4 lightView;
uniform mat4 mvp;
uniform sampler2D depthMap;
uniform sampler2D normalMap;
uniform sampler2D posMap;

out vec3 destColor;
out vec3 testPos;
out vec2 uv;
out float render;

vec3 worldLightPos = vec3(5.0, 2.0, -3.0);
float eta = 1.0/1.33;

void main()
{
    // world pos
    vec4 worldPos = translate * vec4(pos, 1);
    render = 1.0;

    // particle projected into light view
    vec4 lvpPos = lvp * translate * vec4(pos, 1);
    gl_Position = lvpPos;

    // get the normal for this particle
    vec2 uv = lvpPos.xy / lvpPos.w;
    uv = uv / 2.0 + 0.5;
    vec3 norm = texture2D(normalMap, uv).xyz;

    vec3 viewDir = normalize(vec3(worldLightPos - lvpPos.xyz));
    vec3 refrDir = normalize(refract( -viewDir, norm, eta));

    // initial estimate
    vec3 p1 = worldPos.xyz + refrDir * 1.0; // world space estimate
    vec4 p1_proj = lvp * vec4(p1, 1.0);
    vec2 fetchCoords1 = (p1_proj.xy / p1_proj.w) / 2.0 + 0.5;
    vec4 dest1 = texture2D(posMap, fetchCoords1);
    if (dest1.w == 0.0) {
        render = 0.0;
        return;
    }
    float d1 = length(lvpPos.xyz - dest1.xyx);

    // first (only?) real iteration
    vec3 p2 = worldPos.xyz + refrDir * d1;
    vec4 p2_proj = lvp * vec4(p2, 1.0);
    vec2 fetchCoords2 = (p2_proj.xy / p2_proj.w) / 2.0 + 0.5;
    vec4 dest2 = texture2D(posMap, fetchCoords2);       // this should be the world coordinate of the caustic "spot"
    if (dest2.w == 0.0) {
        render = 0.0;
        return;
    }

    vec4 outPos = mvp * vec4(dest2.xyz, 1.0);
    gl_Position = outPos;
}