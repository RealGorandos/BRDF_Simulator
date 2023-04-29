import Scene.Raster;
import Rendering.Lights.LightHelpers;
import Utils.Sampling.TinyUniformSampleGenerator;
import Utils.Math.MathHelpers;
#include "Simulation.lib.hlsl"
#include "Cook_Torrence.brdf.hlsl"

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
    uniform float3 c_pos;

    uniform float4x4 gWorld;
};



VSOut vsMain(VSIn vIn, out float2 fragCoord  : TEXCOORD0)
{

    vIn.pos.y = random(vIn.pos.xz) * (3 * roughness);
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



void updateTexture(float3 posIn, float3 dirIn) {
    uint twidth;
    uint theight;
    tex2D_uav.GetDimensions(twidth, theight);
   
    int hitCount = bounces - 1;
    float3 pos = posIn;
    float3 dir = dirIn;

    bool render = ray_march(pos, dir, hitCount);
    if (hitCount >= 0 && render) {
        float2 res = world_to_latlong_map(normalize(dir));
        InterlockedAdd(tex2D_uav[uint2((res.x * twidth), (res.y * theight))], 1);
    }

}

float4 psMain(VSOut vsOut) : SV_TARGET
{
    if (BRDF_Simulation) {
        float3 c_dir = normalize(c_pos);
        float3 preRef = normalize(reflect(c_dir, vsOut.normalW));
        
        updateTexture(vsOut.posW, preRef);
        }
    return float4(vsOut.normalW, 1.f);
}
