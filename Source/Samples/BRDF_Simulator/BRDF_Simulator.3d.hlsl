import Scene.Raster;
import Rendering.Lights.LightHelpers;
import Utils.Sampling.TinyUniformSampleGenerator;
import Utils.Math.MathHelpers;

#define PI 3.14159265358979323846264338327950288

//uniform EnvMap gEnvMap : register(t7);
RWStructuredBuffer<int> counter : register(u0);
cbuffer PerFrameCB : register(b0)
{
    bool simulate;
    bool normalSim;
    bool gConstColor;
    RWTexture2D<uint> tex2D_uav;
    SamplerState  envSampler;
    uniform float roughness;
    uniform int samples;
    uniform int bounces;
    uniform int2 surfaceSize;
    uniform float totalPixels;

    uniform int orthCamWidth;
    uniform int orthCamHeight;
    uniform int nearPlanePos;
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
    vIn.pos.y = random(vIn.pos.xz) * (10 * roughness);
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


/*Moller-Trumbore Method*/
float3 rayTrinagleIntersect(float3 V0, float3 V1, float3 V2, float3 O, float3 D) {
    float3 E1 = V1 - V0;
    float3 E2 = V2 - V0;
    float3 T = O - V0;
    float3 P = cross(D, E2);
    float3 Q = cross(T, E1);
    float3x3 M = float3x3(dot(Q, E2), dot(P, T), dot(Q, D));
    float3x3 M_ = transpose(M);
    float3 c = 1/(dot(P, E1));
    float3 tuv = mul(c ,M_);
    return tuv;
}


void ray_march(in float3 rayOrigin, in float3 rayDir)
{
    float3 cur_bounce_start_pos = rayOrigin;
    float3 current_position = rayOrigin;
    float3 current_rayDir = rayDir;
    float3 current_block = floor(current_position);
    float3 temp_current_block = floor(current_position);
    int bouncedRays = bounces;
    while (
            (current_position.x >= 0 && current_position.x < surfaceSize[1]) &&
            (current_position.z >= 0 && current_position.z < surfaceSize[0]) &&
            (current_position.y < 10) &&
            bouncedRays >= 0
        )
    {
        current_position += current_rayDir * 0.2; //Moving ray position
        temp_current_block = floor(current_position); //Storing the current block
        /*Checking if we moved to a new block*/
        if (temp_current_block.x != current_block.x &&
            temp_current_block.z != current_block.z)
        {
            /*If it is a new block update the variables*/
            current_block = temp_current_block;
            /*The vertices of the upper triangle*/
            float3 a = float3(current_block.x, random(float2(current_block.x, current_block.z)) * (10 * roughness), current_block.z);
            float3 b = float3(current_block.x + 1, random(float2(current_block.x + 1, current_block.z)) * (10 * roughness), current_block.z);
            float3 c = float3(current_block.x, random(float2(current_block.x, current_block.z + 1)) * (10 * roughness), current_block.z + 1);
            float3 d = float3(current_block.x + 1, random(float2(current_block.x + 1, current_block.z + 1)) * (10 * roughness), current_block.z + 1);

            float3 tuv1 = rayTrinagleIntersect(b, a, c, cur_bounce_start_pos, current_rayDir);
            float3 tuv2 = rayTrinagleIntersect(b, c, d, cur_bounce_start_pos, current_rayDir);
            /*Checking which side of the quad the ray is in*/
 
            if ((tuv1[0] >= 0 ) && (tuv1[1] >= 0 ) && (tuv1[2] >= 0 ) && (1 - tuv1[1] - tuv1[2] >= 0)) {
                
                //bouncedRays -= 1;
                cur_bounce_start_pos += current_rayDir * tuv1[0];
                current_position = cur_bounce_start_pos;
                //ASK ABOUT THE CORRECT DIRECTION OF  THE NORMALS
                float3 E1 = a - b;
                float3 E2 = c - b; 
                current_rayDir = normalize(cross(E1, E2));
            }
            else if ((tuv2[0] >= 0) && (tuv2[1] >= 0) && (tuv2[2] >= 0 ) && ( 1 - tuv2[1] - tuv2[2] >= 0)) {
                
                //bouncedRays -= 1;
                cur_bounce_start_pos += current_rayDir * tuv2[0];
                current_position = cur_bounce_start_pos;
                //ASK ABOUT THE CORRECT DIRECTION OF  THE NORMALS
                float3 E1 = b - d;
                float3 E2 = c - d;
                current_rayDir = normalize(cross(E1, E2));

            }
        }
    }
    if (bouncedRays >= 0) {
        uint twidth;
        uint theight;
        tex2D_uav.GetDimensions(twidth, theight);
        float2 res = world_to_latlong_map(current_rayDir);
       // InterlockedAdd(tex2D_uav[uint2((res.x * twidth), (res.y * theight))], 1);
        //return float3(current_rayDir);
    }

}
uint base_hash(uint2 p) {
     p = 1103515245U * ((p >> 1U) ^ (p.yx));
     uint h32 = 1103515245U * ((p.x) ^ (p.y >> 3U));
     return h32 ^ (h32 >> 16);
    
}

float2 PseudoRandom2D(in int i) {
     return frac(float2(i * int2(12664745, 9560333)) / exp2(24.0));
}

float4 psMain(VSOut vsOut) : SV_TARGET
{
    if (gConstColor)
    {
        return float4(0, 1, 0, 1);
    }
    else {
        /*Setting Up Monte-Carlo method*/
        //Generating Random Seed
        int seed = int(base_hash(asint(vsOut.posW.xz)));
        /*Sphere space transformation matrix*/
        float3 v_ = normalize(cross(vsOut.posW, vsOut.normalW)); // e.g: (1 ,0 ,0)
        float3 u_ = normalize(cross(v_, vsOut.normalW)); // e.g: (0 ,0 ,1)
        float3x3 M = float3x3(v_, vsOut.normalW, u_); // e.g: [(1 ,0 ,0) , (0 ,1 ,0) , (0 ,0 ,1)]
        float3x3 M_ = transpose(M);
        /*Env map diemensions*/
        uint twidth;
        uint theight;
        tex2D_uav.GetDimensions(twidth, theight);
            if (simulate) {
                        /*Generating normals based on the seed*/
                        for (int i = seed; i < samples + seed; i++) {
                            float2 hl = PseudoRandom2D(i);
                            float ourV_sqrt = sqrt(1. - hl.y * hl.y);
                            float3 L = normalize(mul(M_, float3(ourV_sqrt * cos(2. * PI * hl.x), hl.y, ourV_sqrt * sin(2. * PI * hl.x))));
                            ray_march(vsOut.posW, L);
                        }
                    }
            else if (normalSim) {
                float2 res = world_to_latlong_map(vsOut.normalW);
                for (uint i = 0; i < samples; i++) {
                    InterlockedAdd(tex2D_uav[uint2((res.x * twidth), (res.y * theight))], 1);
                }
            }
            return float4(vsOut.normalW, 1.f);
    }

}
