import Scene.Scene;
import Utils.Math.MathHelpers;


Texture2D gTexture;
SamplerState gSampler;

struct VS_OUTPUT
{
    float4 posH : SV_POSITION;
    float3 dir : NORMAL;
    float2 texCoord : TEXCOORD;
};


cbuffer PerFrameCB
{
    float4x4 gWorld;
    float4x4 gViewMat;
    uniform uint totalPixels;
    float4x4 gProjMat;
    float gScale;
    uniform uint  gSamples;
   // EnvMap gEnvMap;
    Texture2D tex2D_uav;
    SamplerState envSampler;

};

VS_OUTPUT vsMain(float4 posL : POSITION, float2 texCoord: TEXCOORD)
{
    VS_OUTPUT output;
    output.dir = posL.xyz;
    float4 viewPos = mul(gViewMat, mul(gWorld, posL));
    output.posH = mul(gProjMat, viewPos);
    output.posH.xy *= gScale;
    output.posH.z = output.posH.w;
    output.texCoord = texCoord;
    return output;
}

float4 psMain(VS_OUTPUT vsOut) : SV_TARGET
{
    float2 uv = world_to_latlong_map(vsOut.dir);
    float4 color = tex2D_uav.SampleLevel(envSampler, uv, 0.f);
    uint twidth;
    uint theight;
    tex2D_uav.GetDimensions(twidth, theight);
    uint3 res = uint3(asuint(color.r), asuint(color.g), asuint(color.b));
    uint samples = twidth * theight * gSamples;
  //float3 color = gEnvMap.eval(vsOut.dir);

    return float4(float3(res) / samples, 1.f);
    }
