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


float3 resultColor(float u, float v, float3 texPos, Texture2D currTexture, SamplerState currSampler) {

    float3 lightPosition = float3(3.f);
    float3 L = normalize(lightPosition - texPos);
    float distance = length(lightPosition - texPos);
    float attenuation = 1.0 / (distance * distance);
    float3 lightColor = float3(300.0f);
    float3 radiance = lightColor * attenuation;
    float3 albedo = float3(1.f);
    //float2 uv = world_to_latlong_map(omega_o);
    uint twidth;
    uint theight;
    currTexture.GetDimensions(twidth, theight);
    float4 color = currTexture.SampleLevel(currSampler, float2(u, v), 0.f);
    uint3 res = uint3(asuint(color.r), asuint(color.g), asuint(color.b));
    uint samples = twidth * theight * gSamples;

    float3 normColor = float3(res / samples);
   
    return normColor * radiance * albedo;

    //return float3(0.f);
}


float3 efficient_simulation(float3 texPos, float3 norm) {
    float3 N = normalize(norm);

    float3 V = normalize(camPos - texPos); //incoming
    float2 uv_I = world_to_latlong_map(V);
    float phi_I = M_PI * (2.f * saturate(uv_I.x) - 1.f);
    float theta_I = M_PI * saturate(uv_I.y);
    int theta_p = int(acos(dot(V, N)) * 180.f / PI);
    int layerInd = int(theta_p / 9); //Current layer index


   

    float3 omega_o = normalize(reflect(V, N)); //Outcoming
    float2 uv_o = world_to_latlong_map(omega_o);
    float phi_o = M_PI * (2.f * saturate(uv_o.x) - 1.f);
    float theta_o = M_PI * saturate(uv_o.y);

    float phi_diff = phi_I - phi_o;


    float v = acos(dot(omega_o, N)) * M_1_2PI + 0.5f;
    





    float sinThetaI = sqrt(1. - dot(V, N) * dot(V, N));
    float sinThetaO = sqrt(1. - dot(omega_o, N) * dot(omega_o, N));


    float phi_o_ = 0.f;
    float phi_i_ = 0.f;

    float maxCos = 0.;
    if (sinThetaI > 1e-4 && sinThetaO > 1e-4) {
        float sinPhiI = clamp(V.y / sinThetaI, -1, 1);
        float sinPhiO = clamp(omega_o.y / sinThetaO, -1, 1);
        float cosPhiI = clamp(V.x / sinThetaI, -1, 1);
        float cosPhiO = clamp(omega_o.x / sinThetaO, -1, 1);
        float dcos = cosPhiI * cosPhiO - sinPhiI * sinPhiO;
        maxCos = max(0., dcos);


        phi_o_ = asin(sinPhiO);
        phi_i_ = asin(sinPhiI);

    }


    float u = (phi_i_ - phi_o_ ) * M_1_2PI;
    u = uv_o.x;
    v = uv_o.y;

    float3 color = float3(0.f);
   // Texture2D currTexture = texture2d_0;
    SamplerState currSampler = gSampler_0;
    bool exist = false;

    //float3 color = float3(0.f);

    if (layerInd == 0) {
        color = resultColor(u, v, texPos, texture2d_0, gSampler_0);
        //color = float3(1.f);
    }
    else if (layerInd == 1 ) {
        color = resultColor(u, v, texPos, texture2d_1, gSampler_1);
        //color = float3(1.f);
    }
    else if (layerInd == 2) {
        color = resultColor(u, v, texPos, texture2d_2, gSampler_2);
        //color = float3(1.f);
    }
    else if (layerInd ==  3) {

        color = resultColor(u, v, texPos, texture2d_3, gSampler_3);
        //color = float3(1.f);
    }
    else if (layerInd == 4 ) {
        color = resultColor(u, v, texPos, texture2d_4, gSampler_4);
        //color = float3(1.f);
    }
    else if (layerInd == 5 ) {
        color = resultColor(u, v, texPos, texture2d_5, gSampler_5);
    }
    else if (layerInd == 6 ) {
        color = resultColor(u, v, texPos, texture2d_6, gSampler_6);
        //color = float3(1.f);
    }
    else if (layerInd == 7 ) {
        color = resultColor(u, v, texPos, texture2d_7, gSampler_7);
        //color = float3(1.f);
    }
    else if (layerInd == 8 ) {
        color = resultColor(u, v, texPos, texture2d_8, gSampler_8);
        //color = float3(1.f);
    }
    else if (layerInd == 9 ) {
        color = resultColor(u, v, texPos, texture2d_9, gSampler_9);
        //color = float3(1.f);
    }
    else {
        color = float3(0.f);
    }



    return color;
}


