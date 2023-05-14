#define PI 3.14159265359

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float3 cook_torrence_BRDF(float3 texPos, float3 norm) {

    float3 N = normalize(norm);
    float3 V = normalize(camPos - texPos);

    float3 Lo = float3(0.0);
    float3 lightPosition = float3(1.f);
    float3 L = normalize(lightPosition - texPos);
    float3 H = normalize(V + L);
    float distance = length(lightPosition - texPos);
    float attenuation = 1.0 / (distance * distance);
    float3 lightColor = float3(300.f);
    float3 radiance = lightColor * attenuation;


    float3 F0 = float3(0.04);
    F0 = lerp(F0, albedo, metallic);
    float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);

    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    float3 specular = numerator / denominator;

    float3 kS = F;
    float3 kD = float3(1.0) - kS;

    kD *= 1.0 - metallic;

    //const float PI = 3.14159265359;

    float NdotL = max(dot(N, L), 0.0);
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    //TODO: SET AO TO BE ENTERED BY THE USER
   // float3 ambient = float3(0.03) * albedo * ao;
    float3 color = Lo;

    //color = color / (color + float3(1.0));
    //color = pow(color, float3(1.0 / 2.2));
    return color;
}



//float reitz_distribution_GGX(float3 N, float3 H, float a)
//{
//    float a2 = a * a;
//    float NdotH = max(dot(N, H), 0.0);
//    float NdotH2 = NdotH * NdotH;
//
//    float nom = a2;
//    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
//    denom = PI * denom * denom;
//
//    return nom / denom;
//}
//
//
//float schlick_geometry_GGX(float3 N, float3 V, float k)
//{
//    float NdotV = max(dot(N, V), 0.0);
//    float nom = NdotV;
//    float denom = NdotV * (1.0 - k) + k;
//
//    return nom / denom;
//}
//
//float smith_schlick_geometry(float3 N, float3 V, float3 L, float k)
//{
//    return schlick_geometry_GGX(N, V, k) * schlick_geometry_GGX(N, L, k);
//}
//
//float3 schlick_frasnel(float3 N, float3 V, float3 F0)
//{
//    return F0 + (1.0 - F0) * pow(1.0 - dot(N, V), 5.0);
//}
//
//
//float3 render(float3 L, float3 N, float3 V, float distance) {
//    float k = roughness * roughness / 2.; // -->>What is the iParameter0?
//   // float3 envC = texture(skybox, L).rgb; // --> What is this?
//    float3 k_d =  float3(1.f) - F; // -->>What is the iTexture0?
//    float3 PLASTIC_SPECULAR_COLOR = albedo;
//    k_d *= 1.0 - metallic;;
//    float attenuation = 1.0 / (distance * distance);
//    float3 lightColor = float3(5.f);
//    float3 radiance = lightColor * attenuation;
//
//
//    float3 H = normalize(L + V);
//    float3 F = schlick_frasnel(N, V, PLASTIC_SPECULAR_COLOR);
//    float D = reitz_distribution_GGX(N, H, roughness);
//    float G = smith_schlick_geometry(N, V, H, k);
//
//
//    float3 L_r = reflect(-V, N);
//    float3 specular = F * D * G / (4. * dot(L, N) * dot(V, N));
//    float3 diffuse = float3(k_d) * 1. / PI;
//
//
//    //float distance = length(lightPosition - texPos);
//
//    return radiance * (diffuse + specular) * dot(V, N);//(diffuse + specular);
//}
