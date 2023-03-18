import Scene.Scene;
import Utils.Math.MathHelpers;



cbuffer PerFrameCB
{
    float4x4 gWorld;
    float4x4 gViewMat;
    float4x4 gProjMat;
    float gScale;
    //EnvMap gEnvMap;
    Texture2D tex2D_uav;
    SamplerState  envSampler;
};

void vsMain(float4 posL : POSITION, out float3 dir : NORMAL, out float4 posH : SV_POSITION)
{
    dir = posL.xyz;
    float4 viewPos = mul(gViewMat, mul(gWorld, posL));
    posH = mul(gProjMat, viewPos);
    posH.xy *= gScale;
    posH.z = posH.w;
}

float4 psMain(float3 dir : NORMAL, float4 posH : SV_POSITION) : SV_TARGET
{

    float2 res = world_to_latlong_map(dir);
    float3 color = tex2D_uav.SampleLevel(envSampler, res, 0.f).rgb * 1.f;

     return float4(color, 1.f);
}
