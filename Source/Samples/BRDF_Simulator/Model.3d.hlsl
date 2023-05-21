import Scene.Raster;
import Rendering.Lights.LightHelpers;
import Utils.Sampling.TinyUniformSampleGenerator;
import Utils.Math.MathHelpers;
import Scene.Scene;
#include "Cook_Torrence.brdf.hlsl"
#include "Efficient_Simulation.hlsl"

cbuffer PerFrameCB : register(b0)
{

    uniform float3 camPos;
    //Matrial
    uniform float roughness;
    uniform float metallic;
    uniform float ao;
    uniform float3 albedo;
    //Simulation Type
    uniform bool cookTorrence;
    uniform bool mySimulation;

    //brdf simulation boolean
    uniform bool startBrdf;

    uniform uint  gSamples;

    uniform float3 lightIntensity;
    uniform float normalizing;
    uniform float camRes;
    Texture2D texture2d_0;
    Texture2D texture2d_1;
    Texture2D texture2d_2;
    Texture2D texture2d_3;
    Texture2D texture2d_4;
    Texture2D texture2d_5;
    Texture2D texture2d_6;
    Texture2D texture2d_7;
    Texture2D texture2d_8;
    Texture2D texture2d_9;
    Texture2D texture2d_10;
    Texture2D texture2d_11;
    Texture2D texture2d_12;
    Texture2D texture2d_13;
    Texture2D texture2d_14;
   
    SamplerState gSampler_0;
    SamplerState gSampler_1;
    SamplerState gSampler_2;
    SamplerState gSampler_3;
    SamplerState gSampler_4;
    SamplerState gSampler_5;
    SamplerState gSampler_6;
    SamplerState gSampler_7;
    SamplerState gSampler_8;
    SamplerState gSampler_9;
    SamplerState gSampler_10;
    SamplerState gSampler_11;
    SamplerState gSampler_12;
    SamplerState gSampler_13;
    SamplerState gSampler_14;

};

    //Texture2D  textureArr[15];// ({ texture2d_0, texture2d_1, texture2d_2, texture2d_3, texture2d_4, texture2d_5, texture2d_6, texture2d_7, texture2d_8, texture2d_9, texture2d_10, texture2d_11, texture2d_12, texture2d_13,texture2d_14 });
    //SamplerState  samplerArr[15];// { gSampler_0, gSampler_1, gSampler_2, gSampler_3, gSampler_4, gSampler_5, gSampler_6, gSampler_7, gSampler_8, gSampler_9, gSampler_10, gSampler_11, gSampler_12, gSampler_13, gSampler_14 });

VSOut vsMain(VSIn vIn)
{

    VSOut resVS = defaultVS(vIn);
    return resVS;
}





float4 psMain(VSOut vsOut) : SV_TARGET
{
    float3 lightPosition = float3(1.f);
    float3 L = normalize(lightPosition - vsOut.posW);
    float3 N = normalize(vsOut.normalW);
    float3 V = normalize(camPos - vsOut.posW);
    float distance = length(lightPosition - vsOut.posW);
    int theta_p = int(acos(dot(V, N)) * 180.f / M_PI);
    int layerInd = int(theta_p / 9); //Current layer index
    
    float3 fst_axis = normalize(cross(V, N));          // (1,0,0)
    float3 sec_axis = normalize(cross(fst_axis, N));   // (0,0,1)
    float3x3 axis = float3x3(fst_axis, N, sec_axis);   // [(1,0,0), (0,1,0), (0,0,1)] Using right hand style. |/_
    float3x3 inv_axis = transpose(axis);
    float3 dir = mul(L, inv_axis);
    float2 uv = world_to_latlong_map(dir);

    float3 color = float3(0.f);



    
    
    if (startBrdf) {
        if (cookTorrence) {
            color = render(L,N, V, distance);
            return float4(color, 1.f);
        }
        else if (mySimulation) {
            color = efficient_simulation(L, N, V, distance, layerInd, uv);
            return float4(color, 1.f);
        }
    } 
return float4(0.f,0.f,0.f, 1.f);
}
