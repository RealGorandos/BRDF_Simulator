import Scene.Raster;
import Utils.Sampling.TinyUniformSampleGenerator;
import Rendering.Lights.LightHelpers;
import Utils.Math.MathHelpers;


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
};

float3x3 rotate(float alpha, float beta, float gamma) {
    float3x3 yaw = float3x3(float3(cos(alpha), -sin(alpha), 0),
                            float3(sin(alpha), cos(alpha), 0),
                            float3(0, 0, 1));

    float3x3 pitch = float3x3(float3(cos(beta), 0, sin(beta)),
                                float3(0,1 , 0),
                                float3(-sin(beta), 0, cos(beta)));

    float3x3 roll = float3x3(float3(1, 0, 0),
                             float3(0, cos(gamma), -sin(gamma)),
                             float3(0, sin(gamma), cos(gamma)));

    return mul(mul(yaw, pitch), roll) ;
    
}

float4x4 scale(float s) {
    float4x4 sc = float4x4(float4(s, 0, 0, 0),
                            float4(0, s,0, 0),
                            float4(0, 0, s, 0),
                            float4(0, 0, 0, 1)
        );
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
    if (v3.x != 0 || v3.y != 0 || v3.z != 0){ //(a.x != b.x && a.y != b.y && a.z != b.z) || ((a.x != c.x && a.y != c.y && a.z != c.z)) || (c.x != b.x && c.y != b.y && c.z != b.z)) {
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

/*************Helper Functions Section*************/

/*Moller-Trumbore Method*/
bool rayTrinagleIntersect(float3 V0, float3 V1, float3 V2, float3 O, float3 D) {
    float3 E1 = V1 - V0;
    float3 E2 = V2 - V0;
    float3 T = O - V0;
    float3 P = cross(D, E2);
    float3 Q = cross(T, E1);
    float3x3 M = float3x3(dot(Q, E2), dot(P, T), dot(Q, D));
    float3x3 M_ = transpose(M);
    float3 c = 1 / (dot(P, E1));
    float3 tuv = mul(c, M_);
    return (tuv[1] >= 0 && tuv[1] <= 1) && (tuv[2] >= 0 && tuv[2] <= 1) && ((1 - tuv[1] - tuv[2]) >= 0);
    //return tuv;
}
/*Trace Ray*/
bool rayTriangleIntersect(
    const inout float3 orig, const inout float3 dir,
    const inout float3 v0, const inout float3 v1, const inout float3 v2,
    inout float t, inout float u, inout float v)
{
    float3 v0v1 = v1 - v0;
    float3 v0v2 = v2 - v0;
    float3 pvec = cross(dir,v0v2);
    float det = dot(v0v1, pvec);
    // ray and triangle are parallel if det is close to 0
    if (det > -kEpsilon && det < kEpsilon) return false;

    float invDet = 1.0 / det;

    float3 tvec = orig - v0;
    u = mul(dot(tvec, pvec),invDet);
    if (u < 0.f || u > 1.0) return false;

    float3 qvec = cross(tvec, v0v1);
    v = mul(dot(dir, qvec),invDet);
    if (v < 0.f || u + v > 1.0) return false;

    t = mul(dot(v0v2, qvec) , invDet);


    return true;

}


uint hash(uint x) {
    x += (x << 10u);
    x ^= (x >> 6u);
    x += (x << 3u);
    x ^= (x >> 11u);
    x += (x << 15u);
    return x;
}

uint hash(uint2 v) { return hash(v.x ^ hash(v.y)); }

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct(uint m) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = asfloat(m);       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}

float random(float2  v) { return floatConstruct(hash(asuint(v))); }

bool findQuad(inout int row, inout int col, inout float3 pos, inout float3 dir,
    inout float t, inout float u, inout float v,
    inout float3 v1, inout float3 v2, inout float3 v3, inout float3 v4)
{
    row = 0;
    col = 0;
    for (row = 0; row < surfaceSize[0]; row++) {
        for (col = 0; col < surfaceSize[1]; col++) {
            float3 v1 = float3(float(col), random(float2(col, row)) * (10 * roughness), float(row));
            float3 v2 = float3(float(col + 1), random(float2(col + 1, row)) * (10 * roughness), float(row));
            float3 v3 = float3(float(col), random(float2(col, row + 1)) * (10 * roughness), float(row + 1));
            float3 v4 = float3(float(col + 1), random(float2(col + 1, row + 1)) * (10 * roughness), float(row + 1));
            if (rayTriangleIntersect(pos, dir, v1, v2, v3, t, u ,v)) {
                return true;
            }
            else if (rayTriangleIntersect(pos, dir, v2, v3, v4, t ,u ,v)) {
                return true;
            }
        }
    }
    return false;
}

float4 psMain(VS_OUTPUT vsOut) : SV_TARGET
{
       float t = 0.f;
       float u = 0.f;
       float v = 0.f;

   if (simulate && (vsOut.normalW.x != 0 || vsOut.normalW.y != 0 || vsOut.normalW.z != 0) && vsOut.normalW.y < 0) {

       float3 v1 = float3(0.f);
       float3 v2 = float3(0.f);
       float3 v3 = float3(0.f);
       float3 v4 = float3(0.f);
       float3 dir = vsOut.normalW;
       float3 pos = vsOut.posW;
       int row = 0;
       int col = 0;
       bool quad = findQuad(row, col, pos, dir, t, u, v, v1, v2, v3, v4);

       if (quad) {
           uint twidth;
           uint theight;
           tex2D_uav.GetDimensions(twidth, theight);
           float2 res = world_to_latlong_map(dir);
           for (uint i = 0; i < 10; i++) {
               InterlockedAdd(tex2D_uav[uint2((res.x * twidth), (res.y * theight))], 1);
           }

       }

   }

   return float4(1,1,1, 1);

}
