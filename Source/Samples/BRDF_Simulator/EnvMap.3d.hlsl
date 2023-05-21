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
    float4x4 gProjMat;
    float gScale;
    uniform uint  gSamples;
    // EnvMap gEnvMap;
    Texture2D tex2D_uav;
    SamplerState envSampler;
    uniform float camRes;


};

VS_OUTPUT vsMain(float4 posL : POSITION, float2 texCoord : TEXCOORD)
{
    VS_OUTPUT output;
    float4 pos = posL;// float4(posL.x + 1.5, posL.y, posL.z + 1.5, posL.w);
    output.dir = pos.xyz;
    float4 viewPos = mul(gViewMat, mul(gWorld, pos));
    output.posH = mul(gProjMat, viewPos);
    output.posH.xy *= gScale;
    output.posH.z = output.posH.w;
    output.texCoord = texCoord;
    return output;
}

float4 psMain(VS_OUTPUT vsOut) : SV_TARGET
{

        float2 uv = world_to_latlong_map(vsOut.dir);
        float4 color = tex2D_uav.Sample(envSampler, uv);
        uint twidth;
        uint theight;
        tex2D_uav.GetDimensions(twidth, theight);


        uint3 res = uint3(asuint(color.r), asuint(color.g), asuint(color.b));
        uint samples = camRes * gSamples;

          return float4(float3(res) / samples, 1.f);


}
