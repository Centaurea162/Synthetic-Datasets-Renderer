#version 330 core
const float PI = 3.14159265359;
out vec4 FragColor;


in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 nDirWS;    //法线方向
    vec3 tDirWS;    //切线方向
    vec3 bDirWS;    //副切线方向
} fs_in;


uniform sampler2D TexBaseColor1;
uniform sampler2D TexMetalness1;
uniform sampler2D TexNormal1;
uniform sampler2D TexRoughness1;

uniform vec3 lightDirections[3];
uniform float intensity[3];
uniform float metalnessFactor;
uniform float roughnessFactor;
uniform float ambient;

uniform vec3 cameraPos;


struct Light
{
    vec3 color;
    vec3 direction;
};

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(TexNormal1, fs_in.TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(fs_in.FragPos);
    vec3 Q2  = dFdy(fs_in.FragPos);
    vec2 st1 = dFdx(fs_in.TexCoords);
    vec2 st2 = dFdy(fs_in.TexCoords);

    vec3 N   = normalize(fs_in.nDirWS);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}


// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}


void main()
{
//    vec3 BaseColor = pow(texture(TexBaseColor1, fs_in.TexCoords).rgb, vec3(2.2));
    vec3 BaseColor = pow(vec3(1.0), vec3(2.2));
    float metallic = 1.0f * metalnessFactor;
    float roughness = 1.0f * roughnessFactor;
//    float metallic = min(texture(TexMetalness1, fs_in.TexCoords).r * metalnessFactor, 1.0);
//    float roughness  = min(texture(TexRoughness1, fs_in.TexCoords).r * roughnessFactor, 1.0);

    vec3 N = fs_in.nDirWS;
    vec3 V = normalize(cameraPos - fs_in.FragPos);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, BaseColor, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i)
    {


        // calculate per-light radiance
        vec3 L = normalize(lightDirections[i]);
        vec3 H = normalize(V + L);
//        float distance = 1.0;
//        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = vec3(intensity[i]);

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;

        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);

        // add to outgoing radiance Lo
        Lo += (kD * BaseColor / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF
    }

    // ambient lighting (note that the next IBL tutorial will replace
    // this ambient lighting with environment lighting).
    vec3 color = vec3(ambient) * BaseColor + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(vec3(color), 1.0);

    
}