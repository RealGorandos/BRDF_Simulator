#define PI 3.14159265359
#include "Utils/Math/MathConstants.slangh"


//float3 getColor(float3 texPos, float3 norm, Texture2D currLayer, SamplerState currSampler) {
//    float3 N = normalize(norm);
//    float3 V = normalize(camPos - texPos);
//
//    float3 lightPosition = float3(3.f);
//    float3 L = normalize(lightPosition - texPos);
//    float distance = length(lightPosition - texPos);
//    float attenuation = 1.0 / (distance * distance);
//    float3 lightColor = float3(300.0f);
//    float3 radiance = lightColor * attenuation;
//
//    float3 fst_axis = normalize(cross(V, N));          // (1,0,0)
//    float3 sec_axis = normalize(cross(fst_axis, N));   // (0,0,1)
//    float3x3 axis = float3x3(fst_axis, N, sec_axis);          // [(1,0,0), (0,1,0), (0,0,1)] Using right hand style. |/_
//    float3x3 inv_axis = transpose(axis);
//
//    float3 ref = normalize(reflect(V, N));
//    float3 dir = mul(inv_axis, ref);
//    float2 uv = world_to_latlong_map(dir);
//    float3 albedo = float3(1.f);
//
//    uint twidth;
//    uint theight;
//    currLayer.GetDimensions(twidth, theight);
//
//    float3 color = currLayer[uint2((uv.x * twidth), (uv.y * theight))].rgb * radiance * albedo;
//        //(currLayer.SampleLevel(currSampler, -uv, 0.f).rgb / gSamples);// *gSamples* radiance* albedo;
//    return color;
//}

//float3 render(float3 texPos, float3 norm) {
//    float3 N = normalize(norm);
//    float3 V = normalize(camPos - texPos);
//    int theta_p = int(acos(dot(V, N)) * 180.f / PI);
//    int layerInd = theta_p / 9;
//
//    float3 color = float3(0.f);
//    if (layerInd == 0) {
//        float3 N = normalize(norm);
//        float3 V = normalize(camPos - texPos);
//
//        float3 lightPosition = float3(3.f);
//        float3 L = normalize(lightPosition - texPos);
//        float distance = length(lightPosition - texPos);
//        float attenuation = 1.0 / (distance * distance);
//        float3 lightColor = float3(300.0f);
//        float3 radiance = lightColor * attenuation;
//
//        float3 fst_axis = normalize(cross(V, N));          // (1,0,0)
//        float3 sec_axis = normalize(cross(fst_axis, N));   // (0,0,1)
//        float3x3 axis = float3x3(fst_axis, N, sec_axis);          // [(1,0,0), (0,1,0), (0,0,1)] Using right hand style. |/_
//        float3x3 inv_axis = transpose(axis);
//
//        float3 ref = normalize(reflect(V, N));
//        float3 dir = mul(inv_axis, ref);
//        float2 uv = world_to_latlong_map(dir);
//        float3 albedo = float3(1.f);
//
//        uint twidth;
//        uint theight;
//        texture2d_0.GetDimensions(twidth, theight);
//
//        texture2d_0[uint2((uv.x * twidth), (uv.y * theight))]+=1.f;// .rgb* radiance* albedo;
//        //color = getColor(texPos, norm, texture2d_0, gSampler_0);
//    }
//    else if (layerInd == 1 ) {
//        float3 N = normalize(norm);
//        float3 V = normalize(camPos - texPos);
//
//        float3 lightPosition = float3(3.f);
//        float3 L = normalize(lightPosition - texPos);
//        float distance = length(lightPosition - texPos);
//        float attenuation = 1.0 / (distance * distance);
//        float3 lightColor = float3(300.0f);
//        float3 radiance = lightColor * attenuation;
//
//        float3 fst_axis = normalize(cross(V, N));          // (1,0,0)
//        float3 sec_axis = normalize(cross(fst_axis, N));   // (0,0,1)
//        float3x3 axis = float3x3(fst_axis, N, sec_axis);          // [(1,0,0), (0,1,0), (0,0,1)] Using right hand style. |/_
//        float3x3 inv_axis = transpose(axis);
//
//        float3 ref = normalize(reflect(V, N));
//        float3 dir = mul(inv_axis, ref);
//        float2 uv = world_to_latlong_map(dir);
//        float3 albedo = float3(1.f);
//
//        uint twidth;
//        uint theight;
//        texture2d_1.GetDimensions(twidth, theight);
//
//         texture2d_1[uint2((uv.x * twidth), (uv.y * theight))]+= 1.f;// .rgb* radiance* albedo;
//      //  color = getColor(texPos, norm, texture2d_1, gSampler_1);
//    }
//    else if (layerInd == 2) {
//            float3 N = normalize(norm);
//            float3 V = normalize(camPos - texPos);
//
//            float3 lightPosition = float3(3.f);
//            float3 L = normalize(lightPosition - texPos);
//            float distance = length(lightPosition - texPos);
//            float attenuation = 1.0 / (distance * distance);
//            float3 lightColor = float3(300.0f);
//            float3 radiance = lightColor * attenuation;
//
//            float3 fst_axis = normalize(cross(V, N));          // (1,0,0)
//            float3 sec_axis = normalize(cross(fst_axis, N));   // (0,0,1)
//            float3x3 axis = float3x3(fst_axis, N, sec_axis);          // [(1,0,0), (0,1,0), (0,0,1)] Using right hand style. |/_
//            float3x3 inv_axis = transpose(axis);
//
//            float3 ref = normalize(reflect(V, N));
//            float3 dir = mul(inv_axis, ref);
//            float2 uv = world_to_latlong_map(dir);
//            float3 albedo = float3(1.f);
//
//            uint twidth;
//            uint theight;
//            texture2d_2.GetDimensions(twidth, theight);
//
//            texture2d_2[uint2((uv.x * twidth), (uv.y * theight))]+= 1.f;// .rgb* radiance* albedo;
//    }
//    else if (layerInd ==  3) {
//            float3 N = normalize(norm);
//            float3 V = normalize(camPos - texPos);
//
//            float3 lightPosition = float3(3.f);
//            float3 L = normalize(lightPosition - texPos);
//            float distance = length(lightPosition - texPos);
//            float attenuation = 1.0 / (distance * distance);
//            float3 lightColor = float3(300.0f);
//            float3 radiance = lightColor * attenuation;
//
//            float3 fst_axis = normalize(cross(V, N));          // (1,0,0)
//            float3 sec_axis = normalize(cross(fst_axis, N));   // (0,0,1)
//            float3x3 axis = float3x3(fst_axis, N, sec_axis);          // [(1,0,0), (0,1,0), (0,0,1)] Using right hand style. |/_
//            float3x3 inv_axis = transpose(axis);
//
//            float3 ref = normalize(reflect(V, N));
//            float3 dir = mul(inv_axis, ref);
//            float2 uv = world_to_latlong_map(dir);
//            float3 albedo = float3(1.f);
//
//            uint twidth;
//            uint theight;
//            texture2d_3.GetDimensions(twidth, theight);
//
//            texture2d_3[uint2((uv.x * twidth), (uv.y * theight))]+= 1.f;// .rgb* radiance* albedo;
//    }
//    else if (layerInd == 4 ) {
//        float3 N = normalize(norm);
//        float3 V = normalize(camPos - texPos);
//
//        float3 lightPosition = float3(3.f);
//        float3 L = normalize(lightPosition - texPos);
//        float distance = length(lightPosition - texPos);
//        float attenuation = 1.0 / (distance * distance);
//        float3 lightColor = float3(300.0f);
//        float3 radiance = lightColor * attenuation;
//
//        float3 fst_axis = normalize(cross(V, N));          // (1,0,0)
//        float3 sec_axis = normalize(cross(fst_axis, N));   // (0,0,1)
//        float3x3 axis = float3x3(fst_axis, N, sec_axis);          // [(1,0,0), (0,1,0), (0,0,1)] Using right hand style. |/_
//        float3x3 inv_axis = transpose(axis);
//
//        float3 ref = normalize(reflect(V, N));
//        float3 dir = mul(inv_axis, ref);
//        float2 uv = world_to_latlong_map(dir);
//        float3 albedo = float3(1.f);
//
//        uint twidth;
//        uint theight;
//        texture2d_4.GetDimensions(twidth, theight);
//
//        texture2d_4[uint2((uv.x * twidth), (uv.y * theight))]+= 1.f;// .rgb* radiance* albedo;
//    }
//    else if (layerInd == 5 ) {
//        float3 N = normalize(norm);
//        float3 V = normalize(camPos - texPos);
//
//        float3 lightPosition = float3(3.f);
//        float3 L = normalize(lightPosition - texPos);
//        float distance = length(lightPosition - texPos);
//        float attenuation = 1.0 / (distance * distance);
//        float3 lightColor = float3(300.0f);
//        float3 radiance = lightColor * attenuation;
//
//        float3 fst_axis = normalize(cross(V, N));          // (1,0,0)
//        float3 sec_axis = normalize(cross(fst_axis, N));   // (0,0,1)
//        float3x3 axis = float3x3(fst_axis, N, sec_axis);          // [(1,0,0), (0,1,0), (0,0,1)] Using right hand style. |/_
//        float3x3 inv_axis = transpose(axis);
//
//        float3 ref = normalize(reflect(V, N));
//        float3 dir = mul(inv_axis, ref);
//        float2 uv = world_to_latlong_map(dir);
//        float3 albedo = float3(1.f);
//
//        uint twidth;
//        uint theight;
//        texture2d_5.GetDimensions(twidth, theight);
//
//        texture2d_5[uint2((uv.x * twidth), (uv.y * theight))]+= 1.f;// .rgb* radiance* albedo;
//    }
//    else if (layerInd == 6 ) {
//        float3 N = normalize(norm);
//        float3 V = normalize(camPos - texPos);
//
//        float3 lightPosition = float3(3.f);
//        float3 L = normalize(lightPosition - texPos);
//        float distance = length(lightPosition - texPos);
//        float attenuation = 1.0 / (distance * distance);
//        float3 lightColor = float3(300.0f);
//        float3 radiance = lightColor * attenuation;
//
//        float3 fst_axis = normalize(cross(V, N));          // (1,0,0)
//        float3 sec_axis = normalize(cross(fst_axis, N));   // (0,0,1)
//        float3x3 axis = float3x3(fst_axis, N, sec_axis);          // [(1,0,0), (0,1,0), (0,0,1)] Using right hand style. |/_
//        float3x3 inv_axis = transpose(axis);
//
//        float3 ref = normalize(reflect(V, N));
//        float3 dir = mul(inv_axis, ref);
//        float2 uv = world_to_latlong_map(dir);
//        float3 albedo = float3(1.f);
//
//        uint twidth;
//        uint theight;
//        texture2d_6.GetDimensions(twidth, theight);
//
//        texture2d_6[uint2((uv.x * twidth), (uv.y * theight))] += 1.f;// .rgb* radiance* albedo;
//    }
//    else if (layerInd == 7 ) {
//        float3 N = normalize(norm);
//        float3 V = normalize(camPos - texPos);
//
//        float3 lightPosition = float3(3.f);
//        float3 L = normalize(lightPosition - texPos);
//        float distance = length(lightPosition - texPos);
//        float attenuation = 1.0 / (distance * distance);
//        float3 lightColor = float3(300.0f);
//        float3 radiance = lightColor * attenuation;
//
//        float3 fst_axis = normalize(cross(V, N));          // (1,0,0)
//        float3 sec_axis = normalize(cross(fst_axis, N));   // (0,0,1)
//        float3x3 axis = float3x3(fst_axis, N, sec_axis);          // [(1,0,0), (0,1,0), (0,0,1)] Using right hand style. |/_
//        float3x3 inv_axis = transpose(axis);
//
//        float3 ref = normalize(reflect(V, N));
//        float3 dir = mul(inv_axis, ref);
//        float2 uv = world_to_latlong_map(dir);
//        float3 albedo = float3(1.f);
//
//        uint twidth;
//        uint theight;
//        texture2d_7.GetDimensions(twidth, theight);
//
//        texture2d_7[uint2((uv.x * twidth), (uv.y * theight))] += 1.f;// .rgb* radiance* albedo;
//    }
//    else if (layerInd == 8 ) {
//        float3 N = normalize(norm);
//        float3 V = normalize(camPos - texPos);
//
//        float3 lightPosition = float3(3.f);
//        float3 L = normalize(lightPosition - texPos);
//        float distance = length(lightPosition - texPos);
//        float attenuation = 1.0 / (distance * distance);
//        float3 lightColor = float3(300.0f);
//        float3 radiance = lightColor * attenuation;
//
//        float3 fst_axis = normalize(cross(V, N));          // (1,0,0)
//        float3 sec_axis = normalize(cross(fst_axis, N));   // (0,0,1)
//        float3x3 axis = float3x3(fst_axis, N, sec_axis);          // [(1,0,0), (0,1,0), (0,0,1)] Using right hand style. |/_
//        float3x3 inv_axis = transpose(axis);
//
//        float3 ref = normalize(reflect(V, N));
//        float3 dir = mul(inv_axis, ref);
//        float2 uv = world_to_latlong_map(dir);
//        float3 albedo = float3(1.f);
//
//        uint twidth;
//        uint theight;
//        texture2d_8.GetDimensions(twidth, theight);
//
//        texture2d_8[uint2((uv.x * twidth), (uv.y * theight))] += 1.f;// .rgb* radiance* albedo;
//    }
//    else if (layerInd == 9 ) {
//        float3 N = normalize(norm);
//        float3 V = normalize(camPos - texPos);
//
//        float3 lightPosition = float3(3.f);
//        float3 L = normalize(lightPosition - texPos);
//        float distance = length(lightPosition - texPos);
//        float attenuation = 1.0 / (distance * distance);
//        float3 lightColor = float3(300.0f);
//        float3 radiance = lightColor * attenuation;
//
//        float3 fst_axis = normalize(cross(V, N));          // (1,0,0)
//        float3 sec_axis = normalize(cross(fst_axis, N));   // (0,0,1)
//        float3x3 axis = float3x3(fst_axis, N, sec_axis);          // [(1,0,0), (0,1,0), (0,0,1)] Using right hand style. |/_
//        float3x3 inv_axis = transpose(axis);
//
//        float3 ref = normalize(reflect(V, N));
//        float3 dir = mul(inv_axis, ref);
//        float2 uv = world_to_latlong_map(dir);
//        float3 albedo = float3(1.f);
//
//        uint twidth;
//        uint theight;
//        texture2d_9.GetDimensions(twidth, theight);
//
//        texture2d_9[uint2((uv.x * twidth), (uv.y * theight))] += 1.f;// * radiance * albedo;
//    }
//    else {
//        color = float3(0.f);
//    }
//    if (color.x == 0.f && color.y == 0.f && color.z == 0.f) {
//        return float3(1.f);
//    }
//    return color;
//    //return currLayer;
//}




float3 resultColor(float3 N, float3 V,float u, float v, float3 texPos, RWTexture2D currTexture, SamplerState currSampler, float3 R ) {
    float3 lightPosition = float3(1.f);

    float distance = length(lightPosition - texPos);
    float attenuation = 1.0 / (distance * distance);
    float3 lightColor = float3(300.0f);
    float3 radiance = lightColor * attenuation;

    uint twidth;
    uint theight;
    currTexture.GetDimensions(twidth, theight);
    float4 color = currTexture[uint2(u * twidth, v * theight)];// += 1.f;
    
    uint3 res = uint3(asuint(color.r), asuint(color.g), asuint(color.b));
    uint samples = twidth * theight * gSamples;

    float NdotL = max(dot(N, R), 0.0);

    float3 normColor = float3(res / (samples * .9f));
  /*  float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    float3 specular = normColor / denominator;*/

    float3 colorRes = normColor * radiance; // * NdotL;
    //colorRes = colorRes / (colorRes + float3(1.0));
    //colorRes = pow(colorRes, float3(1.0 / 2.2));
    //    //normColor.r * radiance * albedo;

    return normColor;// normColor.r* radiance* albedo;;
}


float3 efficient_simulation(float3 texPos, float3 norm) {
    float3 N = normalize(norm);
    float3 lightPosition = float3(1.f);

    float3 R = normalize(lightPosition - texPos);


    float3 V = normalize(camPos - texPos);              // incoming


    //float3 R = normalize(reflect(-V, N));               // Outcoming

    float theta = (acos(dot(V, N))) * M_1_PI;           // V theta

    float3 V_ = normalize(V - dot(V, N) * N);
    float3 R_ = normalize(R - dot(R, N) * N);
    float phi = ((atan2(R_.x, R_.z) - atan2(V_.x, V_.z) + M_PI_2) * M_1_2PI + 0.5f);


    float3 uvuv = latlong_map_to_world(float2(phi, theta));
 

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
        //color = float3(1.f);
    }
    else if (layerInd == 8 ) {
        color = resultColor(N, V, u, v, texPos, texture2d_1, gSampler_1, R);
        //color = float3(1.f);
    }
    else if (layerInd == 7) {
        color = resultColor(N, V, u, v, texPos, texture2d_2, gSampler_2, R);
        //color = float3(1.f);
    }
    else if (layerInd ==  6) {
        color = resultColor(N, V, u, v, texPos, texture2d_3, gSampler_3, R);
        //color = float3(1.f);
    }
    else if (layerInd == 5 ) {
        color = resultColor(N, V, u, v, texPos, texture2d_4, gSampler_4, R);
        //color = float3(1.f);
    }
    else if (layerInd == 4 ) {
        color = resultColor(N, V, u, v, texPos, texture2d_5, gSampler_5, R);
    }
    else if (layerInd == 3 ) {
        color = resultColor(N, V, u, v, texPos, texture2d_6, gSampler_6, R);
        //color = float3(1.f);
    }
    else if (layerInd == 2 ) {
        color = resultColor(N, V, u, v, texPos, texture2d_7, gSampler_7, R);
        //color = float3(1.f);
    }
    else if (layerInd == 1 ) {
        color = resultColor(N, V, u, v, texPos, texture2d_8, gSampler_8, R);
        //color = float3(1.f);
    }
    else if (layerInd == 0 ) {
        color = resultColor(N, V, u, v, texPos, texture2d_9, gSampler_9, R);
        //color = float3(1.f);
    }
    else {
        color = float3(0.f);
    }



    return color;
}


