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

float3 cook_torrence_BRDF(float3 V, float3 N, float3 L, inout float3 kS) {
    float3 H = normalize(V + L);
    float3 F0 = lerp(float3(0.04), albedo, metallic);

    float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);

    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;


    float3 specular = numerator / denominator;
    kS = F;
    return specular;
}


float3 render(float3 L, float3 N, float3 V, float distance) {

    float3 Lo = float3(0.0);
    float3 lightColor = float3(10.f);

    float attenuation = 1.0 / (distance * distance);
    float3 radiance = lightColor * attenuation;
    float NdotL = max(dot(N, L), 0.0);


    float3 kS = float3(0.f);
    float3 specular = float3(0.f);


     specular = cook_torrence_BRDF(V, N, L, kS);
  
    float3 kD = float3(1.0) - kS;

    kD *= 1.0 - metallic;



    Lo += (kD * albedo / PI + specular) * radiance * NdotL;

    float3 color = Lo;

    //color = color / (color + float3(1.0));
    //color = pow(color, float3(1.0 / 2.2));

    return color;
}


