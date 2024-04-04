#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 nDirWS;    //法线方向
    vec3 tDirWS;    //切线方向
    vec3 bDirWS;    //副切线方向
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
    vs_out.FragPos = (model * vec4(aPos, 1.0)).rgb;
    vs_out.TexCoords = aTexCoords;
    vs_out.nDirWS = (model * vec4(aNormal, 1.0)).rgb;
    vs_out.tDirWS = (model * vec4(aTangent, 1.0)).rgb;
    vs_out.bDirWS = (model * vec4(aBitangent, 1.0)).rgb;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}