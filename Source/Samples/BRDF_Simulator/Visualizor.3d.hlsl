#include "Utils/Math/MathConstants.slangh"
import Scene.Raster;
import Utils.Sampling.TinyUniformSampleGenerator;
import Rendering.Lights.LightHelpers;
import Utils.Math.MathHelpers;


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
    float4x4 gProjMat;
    uniform int gSize;
    uniform float deg;

    uniform float scaleFact;
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


    float alpha = deg + M_PI + 0.1;
    float3x3 xRot = float3x3(float3(1, 0, 0),
        float3(0, cos(alpha), -sin(alpha)),
        float3(0, sin(alpha), cos(alpha)));

    
    float3 posIn = mul(float4(vIn.pos, 1.f), scale(scaleFact)).xyz;
    posIn = mul(posIn, xRot);
    const float z = -float(gSize + 2) / 2 + 5.f;

    posIn[1] += z* float(sin(alpha));  // y
    posIn[2] += z* float(cos(alpha)) + float(gSize + 2) / 2;  // z
    
    
    float3 posW = mul(gWorld, float4(posIn, 1.f)).xyz;
    output.posW = posW;
    float4x4 gViewProjMat = mul(gProjMat, gViewMat);
    output.posH = mul(gViewProjMat, float4(posW, 1.f));
    output.texC = vIn.texC;
    output.instanceID = instanceID;
    output.normalW = mul(gScene.getInverseTransposeWorldMatrix(instanceID), vIn.unpack().normal);
    float4 tangent = vIn.unpack().tangent;
    output.tangentW = float4(mul((float3x3)gScene.getWorldMatrix(instanceID), tangent.xyz), tangent.w);

    float3 prevPos = posIn;
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




float4 psMain(VS_OUTPUT vsOut) : SV_TARGET
{

return float4(1,0,0, 1);
}
