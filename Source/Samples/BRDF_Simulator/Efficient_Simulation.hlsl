#define PI 3.14159265359
#include "Utils/Math/MathConstants.slangh"


float3 resultColor(float3 N, float3 V,float u, float v, float3 texPos, RWTexture2D currTexture, SamplerState currSampler, float3 R ) {
    float3 lightPosition = float3(1.f);

    float distance = length(lightPosition - texPos);
    float attenuation = 1.0 / (distance * distance);
    float3 lightColor = float3(300.f);
    float3 radiance = lightColor * attenuation;

    uint twidth;
    uint theight;
    currTexture.GetDimensions(twidth, theight);

    float4 color = currTexture[uint2(u * twidth, v * theight)].xxxx;
    
    uint3 res = uint3(asuint(color.r), asuint(color.g), asuint(color.b));
    uint samples = twidth * theight * gSamples;

    float NdotL = max(dot(N, R), 0.0);

    float3 normColor = float3(res / (samples * normalizing));

    float3 colorRes = (albedo / PI + normColor) * radiance * NdotL;

    //colorRes = colorRes / (colorRes + float3(1.0));
    //colorRes = pow(colorRes, float3(1.0 / 2.2));

    return colorRes;
}


float3 efficient_simulation(float3 texPos, float3 norm) {
    float3 N = normalize(norm);
    float3 lightPosition = float3(1.f);


    float3 V = normalize(camPos - texPos);              // incoming


    float3 R = normalize(lightPosition - texPos);

    //float3 R = normalize(reflect(-V, N));               // Outcoming

    //float theta = (acos(dot(V, N))) * M_1_PI;           // V theta

    //float3 V_ = normalize(V - dot(V, N) * N);
    //float3 R_ = normalize(R - dot(R, N) * N);
    //float phi = ((atan2(R_.x, R_.z) - atan2(V_.x, V_.z)) * M_1_2PI + 0.5f);


    //float3 uvuv = latlong_map_to_world(float2(phi, theta));
 

    int theta_p = int(acos(dot(V, N)) * 180.f / M_PI);
    int layerInd = int(theta_p /9); //Current layer index
    
    //return uvuv;


    ////Approach 2:
    float3 fst_axis = normalize(cross(V, N));          // (1,0,0)
    float3 sec_axis = normalize(cross(fst_axis, N));   // (0,0,1)
    float3x3 axis = float3x3(fst_axis, N, sec_axis);   // [(1,0,0), (0,1,0), (0,0,1)] Using right hand style. |/_
    float3x3 inv_axis = transpose(axis);

    float3 dir = mul(R, inv_axis);
    ////return dir;

    float2 uv = world_to_latlong_map(dir);
    float u = uv.x;
    float v = uv.y;





    float3 color = float3(0.f);

    if (layerInd == 9) {
        color = resultColor(N, V, u, v, texPos, texture2d_0, gSampler_0, R);

    }
    else if (layerInd == 8 ) {
        color = resultColor(N, V, u, v, texPos, texture2d_1, gSampler_1, R);

    }
    else if (layerInd == 7) {
        color = resultColor(N, V, u, v, texPos, texture2d_2, gSampler_2, R);

    }
    else if (layerInd ==  6) {
        color = resultColor(N, V, u, v, texPos, texture2d_3, gSampler_3, R);

    }
    else if (layerInd == 5 ) {
        color = resultColor(N, V, u, v, texPos, texture2d_4, gSampler_4, R);
   
    }
    else if (layerInd == 4 ) {
        color = resultColor(N, V, u, v, texPos, texture2d_5, gSampler_5, R);
    }
    else if (layerInd == 3 ) {
        color = resultColor(N, V, u, v, texPos, texture2d_6, gSampler_6, R);
        
    }
    else if (layerInd == 2 ) {
        color = resultColor(N, V, u, v, texPos, texture2d_7, gSampler_7, R);
        
    }
    else if (layerInd == 1 ) {
        color = resultColor(N, V, u, v, texPos, texture2d_8, gSampler_8, R);

    }
    else if (layerInd == 0 ) {
        color = resultColor(N, V, u, v, texPos, texture2d_9, gSampler_9, R);
     
    }
    else {
        color = float3(0.f);
    }



    return color;
}


