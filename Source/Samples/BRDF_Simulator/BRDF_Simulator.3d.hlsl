import Scene.Raster;
import Rendering.Lights.LightHelpers;
import Utils.Sampling.TinyUniformSampleGenerator;
import Utils.Math.MathHelpers;
#include "Simulation.lib.hlsl"


//uniform EnvMap gEnvMap : register(t7);

cbuffer PerFrameCB : register(b0)
{
    bool BRDF_Simulation;
    RWTexture2D<uint> tex2D_uav;
    uniform float roughness;
    uniform int bounces;
    uniform int surfaceSize;
    uniform float3 c_dir;
};



VSOut vsMain(VSIn vIn, out float2 fragCoord  : TEXCOORD0)
{

    vIn.pos.y = random(vIn.pos.xz) * (10 * roughness);
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



void updateTexture(float3 posIn, float3 dirIn, bool upper, bool lower) {
    uint twidth;
    uint theight;
    tex2D_uav.GetDimensions(twidth, theight);
   
    int hitCount = bounces;
    float3 pos = posIn;
    float3 dir = dirIn;

    //Apply Ray marching
    bool render = ray_march(pos, dir, hitCount, upper, lower);
    float3 dirMirror = float3(-dir.x, dir.y, dir.z);

    if (render) {
        float2 res = world_to_latlong_map(normalize(dir));
        float2 resMirror = world_to_latlong_map(normalize(dirMirror));
        InterlockedAdd(tex2D_uav[uint2((res.x * twidth), (res.y * theight))], 1);
        InterlockedAdd(tex2D_uav[uint2((resMirror.x * twidth), (resMirror.y * theight))], 1);
    }

}

float4 psMain(VSOut vsOut) : SV_TARGET
{
    if (BRDF_Simulation) {
        bool upper = false;
        bool lower = false;
        float3 v1 = float3(0.f);
        float3 v2 = float3(0.f);
        float3 v3 = float3(0.f);
        float3 v4 = float3(0.f);

        float t = 0.f;
        float t0 = 0.f;
        float u = 0.f;
        float v = 0.f;

        float3 current_block = floor(vsOut.posW);
        fillVertices(current_block.x, current_block.z, v1, v2, v3, v4);




        float3 V = normalize(c_dir);
        float3 N = normalize(vsOut.normalW);
        float3 preRef = normalize(reflect(V, N));

        //Checking if there is a Ray-Triangle Intersection in the current block
         upper = rayTriangleIntersect(vsOut.posW, preRef, v1, v2, v3, t, u, v);
         lower = rayTriangleIntersect(vsOut.posW, preRef, v4, v3, v2, t0, u, v);
         updateTexture(vsOut.posW, preRef, upper, lower);

        }
    return float4(vsOut.normalW, 1.f);
}
