import Scene.Raster;
import Rendering.Lights.LightHelpers;
import Utils.Sampling.TinyUniformSampleGenerator;
import Utils.Math.MathHelpers;
#define PI 3.14159265358979323846264338327950288

//uniform EnvMap gEnvMap : register(t7);

cbuffer PerFrameCB : register(b0)
{
    bool gConstColor;
    RWTexture2D<float> tex2D_uav;
    SamplerState  envSampler;
    uniform int roughness;
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
float random(float x) { return floatConstruct(hash(asuint(x))); }
float random(float2  v) { return floatConstruct(hash(asuint(v))); }
//float random(float3  v) { return floatConstruct(hash(asuint(v))); }
//float random(float4  v) { return floatConstruct(hash(asuint(v))); }

VSOut vsMain(VSIn vIn, out float2 fragCoord  : TEXCOORD0)
{
    vIn.pos.y = random(vIn.pos.xz) * roughness;
    VSOut resVS = defaultVS(vIn);
    return resVS;
}

[maxvertexcount(3)]
void gsMain(triangle VSOut input[3], inout TriangleStream<VSOut> output) {
    float3 v1 = input[1].posW - input[0].posW;
    float3 v2 = input[2].posW - input[0].posW;
    float3 normal = normalize(cross(v1, v2));
    // Output vertices with their corresponding normal
    for (int i = 0; i < 3; i++)
    {
        VSOut o = input[i];
        o.normalW = normal;
        output.Append(o);
    }
}


float4 psMain(VSOut vsOut) : SV_TARGET
{
    if (gConstColor)
    {
        return float4(0, 1, 0, 1);
    }
    else {
        uint twidth;
        uint theight;
        tex2D_uav.GetDimensions(twidth, theight);

        float2 res = world_to_latlong_map(vsOut.normalW);
        tex2D_uav[uint2(res.x * twidth, res.y * theight)] = 255.f;

        return float4(vsOut.normalW, 1.f);
    }

}
