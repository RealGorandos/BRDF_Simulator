#define PI 3.14159265359
#include "Utils/Math/MathConstants.slangh"


float3 simulated_BRDF(float3 V, float3 N, float3 L, Texture2D currTexture, SamplerState cuurSampler, float2 uv) {
    float3 brdf_float = float3(0.f);
    uint twidth;
    uint theight;
    currTexture.GetDimensions(twidth, theight);
    brdf_float = currTexture.Sample(cuurSampler, uv).xxx;
    return brdf_float;
}


float3 efficient_simulation(float3 L, float3 N, float3 V, float distance,int layerInd, float2 uv) {

    

    float NdotL = max(dot(N, L), 0.0);
    float attenuation = 1.0 / (distance * distance);
    float3 lightColor = float3(300.f);
    float3 radiance = lightColor * attenuation;

    float3 specular_init = float3(0.f);



    switch (layerInd){
        case  14:
            specular_init = simulated_BRDF(V, N, L, texture2d_0, gSampler_0, uv);
            break;
    
        case  13:
            specular_init = simulated_BRDF(V, N, L, texture2d_1, gSampler_1, uv);
            break;
        case  12:
            specular_init = simulated_BRDF(V, N, L, texture2d_2, gSampler_2, uv);
            break;

        case  11:
            specular_init = simulated_BRDF(V, N, L, texture2d_3, gSampler_3, uv);
            break;
    
        case  10:
            specular_init = simulated_BRDF(V, N, L, texture2d_4, gSampler_4, uv);
            break;

        case 9:
            specular_init = simulated_BRDF(V, N, L, texture2d_5, gSampler_5, uv);
            break;

        case   8:
            specular_init = simulated_BRDF(V, N, L, texture2d_6, gSampler_6, uv);
            break;

        case  7:
            specular_init = simulated_BRDF(V, N, L, texture2d_7, gSampler_7, uv);
            break;

        case  6:
            specular_init = simulated_BRDF(V, N, L, texture2d_8, gSampler_8, uv);
            break;

        case   5:
            specular_init = simulated_BRDF(V, N, L, texture2d_9, gSampler_9, uv);
            break;
    
        case  4:
            specular_init = simulated_BRDF(V, N, L, texture2d_10, gSampler_10, uv);
            break;
    
        case  3:
            specular_init = simulated_BRDF(V, N, L, texture2d_11, gSampler_11, uv);
            break;
    
        case  2:
            specular_init = simulated_BRDF(V, N, L, texture2d_12, gSampler_12, uv);
            break;
    
        case  1:
            specular_init = simulated_BRDF(V, N, L, texture2d_13, gSampler_13, uv);
             break;
    
        case  0:
            specular_init = simulated_BRDF(V, N, L, texture2d_14, gSampler_14, uv);
            break;

        default :
            return float3(0.f);

    }

    uint samples = camRes * gSamples;
    uint3 brdf_int = uint3(asuint(specular_init.r), asuint(specular_init.g), asuint(specular_init.b));
    float3 specular = float3(brdf_int / (samples * normalizing));

    float3 color = (albedo / PI + specular) * radiance * NdotL;

    color = color / (color + float3(1.0));
    color = pow(color, float3(1.0 / 2.2));

    return color;
}


