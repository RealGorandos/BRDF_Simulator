import Scene.Raster;
import Rendering.Lights.LightHelpers;
import Utils.Sampling.TinyUniformSampleGenerator;
import Utils.Math.MathHelpers;
#include "Simulation.lib.hlsl"


//uniform EnvMap gEnvMap : register(t7);
RWStructuredBuffer<int> counter : register(u0);
cbuffer PerFrameCB : register(b0)
{
    bool simulate;
    bool BRDF_Simulation;
    bool gConstColor;
    uniform bool LoadedObj;
    RWTexture2D<uint> tex2D_uav;
    SamplerState  envSampler;
    uniform float roughness;
    uniform int samples;
    uniform int bounces;
    uniform int2 surfaceSize;
    uniform float totalPixels;

    uniform float orthCamWidth;
    uniform float orthCamHeight;
    uniform int nearPlanePos;
};



VSOut vsMain(VSIn vIn, out float2 fragCoord  : TEXCOORD0)
{
    if (!LoadedObj) {
        vIn.pos.y = random(vIn.pos.xz) * (3 * roughness);
    }
    VSOut resVS = defaultVS(vIn);
    return resVS;
}

[maxvertexcount(3)]
void gsMain(triangle VSOut input[3], inout TriangleStream<VSOut> output) {
        float3 v1 = input[1].posW - input[0].posW;
        float3 v2 = input[2].posW - input[0].posW;
        float3 normal = normalize(cross(v1, v2));
        for (int i = 0; i < 3; i++)
        {
            VSOut o = input[i];
            o.normalW = normal;
            output.Append(o);
        }

}

float3 JitteredSample(float3 pos, int nSample)
{
    float3 result;
    float invNSamples = 1.0f / (float)nSample;
    float3 cellSize = 1.0f / (float)nSample;

    // Generate random sample within the cell
    float3 jitter = cellSize * RandomGradient3(nSample);
    result += jitter;


    // Offset the sample to the center of the cell
    result.x += (float)pos.x / (float)nSample;
    result.y += (float)pos.y / (float)nSample;
    result.z += (float)pos.z / (float)nSample;

    return result * invNSamples;
}

void updateTexture(float3 posIn, float3 dirIn) {

    int bias = int(base_hash(asint(posIn.xz)));
    uint twidth;
    uint theight;
    tex2D_uav.GetDimensions(twidth, theight);
    for (int i = bias; i < samples + bias; i++) {
        int hitCount = bounces;
        float3 pos = JitteredSample(posIn, i);
        float3 dir = dirIn;
        ray_march(pos, dir, hitCount);
        if (hitCount > 0) {
      
            float2 res = world_to_latlong_map(normalize(dir));
            InterlockedAdd(tex2D_uav[uint2((res.x * twidth), (res.y * theight))], 1);
    

        }
    }
}

float4 psMain(VSOut vsOut) : SV_TARGET
{
    if (LoadedObj) {
        return float4(0.f, 0.f, 0.f, 1.f);
    }
    if (BRDF_Simulation) {
        updateTexture(vsOut.posW, vsOut.normalW);
       
        }
    return float4(vsOut.normalW, 1.f);
}
