import Scene.Raster;
import Utils.Sampling.TinyUniformSampleGenerator;
import Rendering.Lights.LightHelpers;
import Utils.Math.MathHelpers;
#include "Simulation.lib.hlsl"

#define kEpsilon 0.000001
#define PI 3.14159265358979323846264338327950288
struct VS_OUTPUT
{
    float3 posW       : POSW;
    float4 posH : SV_POSITION;
    float2 texC       : TEXCRD;
    nointerpolation GeometryInstanceID instanceID : INSTANCE_ID;  ///< Geometry instance ID.
    float3 normalW    : NORMAL;      ///< Shading normal in world space (not normalized!).
    float4 tangentW   : TANGENT;
    float4 prevPosH   : PREVPOSH;
};


cbuffer PerFrameCB
{
    float4x4 gWorld;
    float4x4 gViewMat;
    uniform uint totalPixels;
    float4x4 gProjMat;
    //  float gScale;
    bool gConstColor;
    uniform float3 rotation;
    uniform float3 camPosition;
    uniform float cameraSize;
    RWTexture2D<uint> tex2D_uav;
    SamplerState  envSampler;
    bool simulate;
    uniform int2 surfaceSize;
    uniform float roughness;
    uniform int bounces;
    float4x4 surfaceWorldMat;
};

float3x3 rotate(float alpha, float beta, float gamma) {
    float3x3 yaw = float3x3(float3(cos(alpha), -sin(alpha), 0),
        float3(sin(alpha), cos(alpha), 0),
        float3(0, 0, 1));

    float3x3 pitch = float3x3(float3(cos(beta), 0, sin(beta)),
        float3(0, 1, 0),
        float3(-sin(beta), 0, cos(beta)));

    float3x3 roll = float3x3(float3(1, 0, 0),
        float3(0, cos(gamma), -sin(gamma)),
        float3(0, sin(gamma), cos(gamma)));

    return mul(mul(yaw, pitch), roll);

}

float4x4 scale(float s) {
    float4x4 sc = float4x4(float4(s, 0, 0, 0),
        float4(0, s, 0, 0),
        float4(0, 0, s, 0),
        float4(0, 0, 0, 1)
        );
    return sc;
}

float4x4 translation(float3 pos) {
    float4x4 sc = float4x4(float4(1, 0, 0, pos.x),
        float4(0, 1, 0, pos.y),
        float4(0, 0, 1, pos.z),
        float4(0, 0, 0, 1));
    return sc;
}


VS_OUTPUT vsMain(VSIn vIn)
{
    VS_OUTPUT output;
    const GeometryInstanceID instanceID = { vIn.instanceID };
    float4x4 worldMat = gScene.getWorldMatrix(instanceID);
    vIn.pos = mul(vIn.pos, rotate(rotation.x, rotation.y, rotation.z));// vIn.pos.x* cos(rotation) - vIn.pos.y * sin(rotation);
    vIn.pos = mul(float4(vIn.pos, 1.f), scale(cameraSize)).xyz;
    //float new_y = vIn.pos.y * cos(rotation) + vIn.pos.x * sin(rotation);
    float3 posW = mul(gWorld, float4(vIn.pos, 1.f)).xyz;
    posW += camPosition;
    output.posW = posW;
    float4x4 gViewProjMat = mul(gProjMat, gViewMat);
    output.posH = mul(gViewProjMat, float4(posW, 1.f));
    output.texC = vIn.texC;
    output.instanceID = instanceID;
    output.normalW = mul(gScene.getInverseTransposeWorldMatrix(instanceID), vIn.unpack().normal);
    float4 tangent = vIn.unpack().tangent;
    output.tangentW = float4(mul((float3x3)gScene.getWorldMatrix(instanceID), tangent.xyz), tangent.w);

    float3 prevPos = vIn.pos;
    GeometryInstanceData instance = gScene.getGeometryInstance(instanceID);
    if (instance.isDynamic())
    {
        uint prevVertexIndex = gScene.meshes[instance.geometryIndex].prevVbOffset + vIn.vertexID;
        prevPos = gScene.prevVertices[prevVertexIndex].position;
    }
    float3 prevPosW = mul(gScene.getPrevWorldMatrix(instanceID), float4(prevPos, 1.f)).xyz;
    output.prevPosH = mul(gScene.camera.data.prevViewProjMatNoJitter, float4(prevPosW, 1.f));

    return output;
}

[maxvertexcount(3)]
void gsMain(triangle VS_OUTPUT input[3], inout TriangleStream<VS_OUTPUT> output) {
    float3 a = input[0].posW;
    float3 b = input[1].posW;
    float3 c = input[2].posW;
    float3 v1 = b - a;
    float3 v2 = c - a;
    float3 v3 = b - c;
    if (v3.x != 0 || v3.y != 0 || v3.z != 0) { //(a.x != b.x && a.y != b.y && a.z != b.z) || ((a.x != c.x && a.y != c.y && a.z != c.z)) || (c.x != b.x && c.y != b.y && c.z != b.z)) {
        float3 normal = normalize(cross(v1, v2));
        // Output vertices with their corresponding normal
        for (int i = 0; i < 3; i++)
        {
            VS_OUTPUT o = input[i];
            o.normalW = normal;
            // }
            output.Append(o);
        }
    }
    else {
        for (int i = 0; i < 3; i++)
        {
            VS_OUTPUT o = input[i];
            o.normalW = float3(0.f);
            output.Append(o);
        }

    }

}



float4 psMain(VS_OUTPUT vsOut) : SV_TARGET
{
       float t = 0.f;
       float u = 0.f;
       float v = 0.f;
       float3 v1 = float3(0.f);
       float3 v2 = float3(0.f);
       float3 v3 = float3(0.f);
       float3 v4 = float3(0.f);
       float3 dir = vsOut.normalW;
       float3 pos = vsOut.posW;
       int bouncesNum = bounces;
       bool render = true;

      if (simulate) {
          bool quad = findQuad( pos, dir, t,  u,  v, v1,  v2, v3, v4);
          if (quad) {
              int hitcount = bounces;
               ray_march(pos, dir, hitcount);
                uint twidth;
                uint theight;
                tex2D_uav.GetDimensions(twidth, theight);
                float2 res = world_to_latlong_map(normalize(dir));
                    for (uint i = 0; i < 10; i++) {
                        InterlockedAdd(tex2D_uav[uint2((res.x * twidth), (res.y * theight))], 1);
                    }
            }




       }
return float4(1, 1, 1, 1);
}
