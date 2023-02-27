import Scene.Raster;
import Rendering.Lights.LightHelpers;
import Utils.Sampling.TinyUniformSampleGenerator;
  
#define PI 3.14159265358979323846264338327950288
cbuffer PerFrameCB : register(b0)
{
    bool gConstColor;
};
// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash(uint x) {
    x += (x << 10u);
    x ^= (x >> 6u);
    x += (x << 3u);
    x ^= (x >> 11u);
    x += (x << 15u);
    return x;
}



// Compound versions of the hashing algorithm I whipped together.
uint hash(uint2 v) { return hash(v.x ^ hash(v.y)); }
//uint hash(uint3 v) { return hash(v.x ^ hash(v.y) ^ hash(v.z)); }
//uint hash(uint4 v) { return hash(v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w)); }



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



// Pseudo-random value in half-open range [0:1].
//float random(float x) { return floatConstruct(hash(asuint(x))); }
float random(float2  v) { return floatConstruct(hash(asuint(v))); }
//float random(float3  v) { return floatConstruct(hash(asuint(v))); }
//float random(float4  v) { return floatConstruct(hash(asuint(v))); }

VSOut vsMain(VSIn vIn)
{
    VSOut resVS = defaultVS(vIn);
    //Generating random seeds based on each vertex xy and yz position
    float u = random(resVS.posW.xy);
    float v = random(resVS.posW.yz);

    //Generating a random vector 
    float x = sqrt(1. - u * u) * cos(2. * PI * v);
    float y = u;
    float z = sqrt(1 - u * u) * sin(2. * PI * v);
    float3 sphereRandomPoint = float3(x, y, z);



    //Generating the transform matrix for the generated normal
    float3 v_ = normalize(cross(resVS.posW, resVS.normalW)); // e.g: (1 ,0 ,0)
    float3 u_ = normalize(cross(v_, resVS.normalW)); // e.g: (0 ,0 ,1)
    float3x3 M = float3x3(v_, resVS.normalW, u_); // e.g: [(1 ,0 ,0) , (0 ,1 ,0) , (0 ,0 ,1)]
    float3x3 M_ = transpose(M);

    //Transforming the normal
    float3 finalNormal = mul(M_ , sphereRandomPoint);
    resVS.normalW = finalNormal;//finalNormal;


    return resVS;
}


float4 psMain(VSOut vsOut, uint triangleIndex : SV_PrimitiveID) : SV_TARGET
{
        return float4(vsOut.normalW, 1.f);
}
