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

    RWTexture2D texture2d_0;
    RWTexture2D texture2d_1;
    RWTexture2D texture2d_2;
    RWTexture2D texture2d_3;
    RWTexture2D texture2d_4;
    RWTexture2D texture2d_5;
    RWTexture2D texture2d_6;
    RWTexture2D texture2d_7;
    RWTexture2D texture2d_8;
    RWTexture2D texture2d_9;
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
};


VSOut vsMain(VSIn vIn)
{

    VSOut resVS = defaultVS(vIn);
    return resVS;
}


float4 psMain(VSOut vsOut) : SV_TARGET
{
    if (startBrdf) {
        if (cookTorrence) {
            float3 color = cook_torrence_BRDF(vsOut.posW, vsOut.normalW);
            return float4(color, 1.f);
        }
        else if (mySimulation) {
            float3 color = efficient_simulation(vsOut.posW, vsOut.normalW);
            return float4(color, 1.f);
        }
    } 
return float4(0.f,0.f,0.f, 1.f);
}
