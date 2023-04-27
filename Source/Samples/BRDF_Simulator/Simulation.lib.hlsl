#define PI 3.14159265358979323846264338327950288
#define kEpsilon 0.00000000001

/*________Helper Methods for finding Quads and Lines_______________________*/

void fillVertices(const int x, const int y, inout float3 v1, inout float3 v2, inout float3 v3, inout float3 v4, float4x4 worldMat, float3x3 invWorldMat) {

    //float3x3 gWorldInv = (invWorldMat);

    //float3 posWv1 = mul(gWorldInv, float3(float(x), 0.f, float(y))).xyz;
    //posWv1.y = random(posWv1.xz) * (3 * roughness);
    //v1 = mul(worldMat, float4(posWv1, 1.f)).xyz;

    //float3 posWv2 = mul(gWorldInv, float3(float(x) + 1.f, 0.f, float(y))).xyz;
    //posWv2.y = random(posWv2.xz) * (3 * roughness);
    //v2 = mul(worldMat, float4(posWv2, 1.f)).xyz;

    //float3 posWv3 = mul(gWorldInv, float3(float(x), 0.f, float(y) + 1.f)).xyz;
    //posWv3.y = random(posWv3.xz) * (3 * roughness);
    //v3 = mul(worldMat, float4(posWv3, 1.f)).xyz;

    //float3 posWv4 = mul(gWorldInv, float3(float(x) + 1.f, 0.f, float(y) + 1.f)).xyz;
    //posWv4.y = random(posWv4.xz) * (3 * roughness);
    //v4 = mul(worldMat, float4(posWv4, 1.f)).xyz;


    v1 = float3(float(x), float(random(float2(x, y)) * (float(3) * roughness)), float(y));

    v2 = float3(float(x) + 1.f, float(random(float2(x + 1.f, y)) * (float(3) * roughness)), float(y));

    v3 = float3(float(x), float(random(float2(x, y + 1.f)) * (float(3) * roughness)), float(y) + 1.f);

    v4 = float3(float(x) + 1.f, float(random(float2(x + 1.f, y + 1.f)) * (float(3) * roughness)), float(y) + 1.f);
}

void findLines(const float3 pos, const float3 dir, const float3 v1, const float3 v2, const float3 v3, const float3 v4, inout float4 firstLine, inout float4 secondLine) {
    //if (pos.x == v2.x) {
    //    firstLine = float4(v2.x + 1, -0.5f, v2.x + 1, float(surfaceSize[0]) - 0.5f);
    //}
    if (dir.x > 0.f) {//This works
        if (pos.x < v2.x) {
            firstLine = float4(v2.x, 1.f, v2.x, float(surfaceSize[0]) + 1);
            // return true;
        }
        else if (pos.x == v2.x) {
            firstLine = float4(v2.x + 1.f, 1.f, v2.x + 1.f, float(surfaceSize[0]) + 1);
        }
    }
    else if (dir.x < 0.f) {
        if (pos.x > v1.x) {
            firstLine = float4(v1.x, 1.f, v1.x, float(surfaceSize[0]) + 1);
        }
        else if (pos.x == v1.x) {
            firstLine = float4(v1.x - 1.f, 1.f, v1.x - 1.f, float(surfaceSize[0]) + 1);
        }
    }

    if (dir.z > 0.f) {
        if (pos.z < v3.z) {
            secondLine = float4(1.f, v3.z, float(surfaceSize[1]) + 1, v3.z);
        }
        else if (pos.z == v3.z) {
            secondLine = float4(1.f, v3.z + 1.f, float(surfaceSize[1]) + 1, v3.z + 1.f);
        }

    }
    else if (dir.z < 0.f) {
        if (pos.z > v1.z) {
            secondLine = float4(1.f, v1.z, float(surfaceSize[1]) + 1, v1.z);
        }
        else if (pos.z == v1.z) {
            secondLine = float4(1.f, v1.z - 1.f, float(surfaceSize[1]) + 1, v1.z - 1.f);
        }
    }

}



//bool findQuad(inout float3 pos, inout float3 dir,
//    inout float t, inout float u, inout float v,
//    inout float3 v1, inout float3 v2, inout float3 v3, inout float3 v4)
//{
//    float4 firstLine = float4(0.f);
//    float4 secondLine = float4(0.f);
//    float t1 = 0.f;
//    float t2 = 0.f;
//    for (int row = 1; row <= surfaceSize[0]; row++) {
//        for (int col = 1; col <= surfaceSize[1]; col++) {
//
//            fillVertices(col, row, v1, v2, v3, v4);
//            //findLines(pos, dir, v1, v2, v3, v4, firstLine, secondLine);
//            //  bool quad = false;
//            //  bool intersection_1 =  rayLineIntersect(float2(pos.x, pos.z), float2(dir.x, dir.z), float2(firstLine[0], firstLine[1]), float2(firstLine[2], firstLine[3]), t1);
//            //  bool intersection_2 = false;// rayLineIntersect(float2(pos.x, pos.z), float2(dir.x, dir.z), float2(2.f, 2.f), float2(2, 3), t2);
//            //    if (intersection_1 || intersection_2) {
//
//            //    float3 v1v2 = v2 - v1;
//            //    float3 v1v3 = v3 - v1;
//            //    float3 N = normalize(cross(v1v3, v1v2));
//            //    //if (t1 < t2 && t1 != 0.f) {
//            //    //    pos += dir * (t1);
//            //    //}
//            //    // if (t2 < t1 && t2 != 0.f) {
//            //        pos += dir * (t1);
//            //    //}
//            //    
//            //    //dir = N;
//
//            //    return true;
//            //}
//            if (rayTriangleIntersect(pos, dir, v1, v2, v3, t, u, v)) {
//                pos = pos + dir * t;
//                float3 v1v2 = v2 - v1;
//                float3 v1v3 = v3 - v1;
//                float3 N = normalize(cross(v1v3, v1v2));
//                //float3 dirOut = dir - N * 2 * dot(dir, N);
//                dir = N;
//                return true;
//            }
//            if (rayTriangleIntersect(pos, dir, v4, v2, v3, t, u, v)) {
//                pos = pos + dir * t;
//                float3 v4v2 = v2 - v4;
//                float3 v4v3 = v3 - v4;
//                float3 N = normalize(cross(v4v2, v4v3));
//                //float3 dirOut = dir - N * 2 * dot(dir, N);
//                dir = N;
//                return true;
//            }
//        }
//    }
//    return false;
//}


/*____________________Microfactes Random Height Generator functions_________________________*/



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



/*___________________Random normal generator Functions (For monte-carlo method)________________*/

uint base_hash(uint2 p) {
    p = 1103515245U * ((p >> 1U) ^ (p.yx));
    uint h32 = 1103515245U * ((p.x) ^ (p.y >> 3U));
    return h32 ^ (h32 >> 16);

}

float2 PseudoRandom2D(in int i) {
    return frac(float2(i * int2(12664745, 9560333)) / exp2(24.0));
}



/*________________Intersection Functions (Trianle/ray and Line/ray)______________________________*/

/*Moller-Trumbore Method*/
bool rayTriangleIntersect(
    const inout float3 orig, const inout float3 dir,
    const inout float3 v0, const inout float3 v1, const inout float3 v2,
    inout float t, inout float u, inout float v)
{
    float3 v0v1 = v1 - v0;
    float3 v0v2 = v2 - v0;
    float3 pvec = cross(dir, v0v2);
    float det = dot(v0v1, pvec);
    // ray and triangle are parallel if det is close to 0
    if ((det > -kEpsilon) && ( det < kEpsilon)) return false;

    float invDet = 1.0 / det;

    float3 tvec = orig - v0;
    u = dot(tvec, pvec) *invDet;
    if (u < 0.f || u > 1.f) return false;

    float3 qvec = cross(tvec, v0v1);
    v = dot(dir, qvec) *invDet;
    if (v < 0.f || u + v > 1.f) return false;

    t = dot(v0v2, qvec) *invDet;
    if (t >= 0.f) return false;
   
    


    return true;

}
/*Ray Line intersection Function*/
bool rayLineIntersect(const float2 rayOrigin, const float2 rayDirection, const float2 lineStart, const float2 lineEnd, inout float t) {


    float2 lineDir = lineEnd - lineStart;

    float d = rayDirection.x * lineDir.y - rayDirection.y * lineDir.x;


    if (d == 0.f) return false;

    float u = ((lineStart.x - rayOrigin.x) * rayDirection.y - (lineStart.y - rayOrigin.y) * rayDirection.x) / d;
    t = ((lineStart.x - rayOrigin.x) * lineDir.y - (lineStart.y - rayOrigin.y) * lineDir.x) / d;

    if ((u < 0.f || u > 1.f) || (t < 0.f)) return false;

    return true;
}


/*_________________________________Ray marching function (For the simulation)___________________*/

void updateNorm_1(inout float3  exp_point, inout float3  dir, float t, float3 v1, float3 v2, float3 v3 ) {
    float3 v1v2 = (v1 - v2);
    float3 v1v3 = (v1 - v3);
    float3 N = normalize(cross(v1v3, v1v2));
  //  float3 dirOut = dir - N * 2 * dot(dir, N);
    exp_point = exp_point + dir * (t);
    //dir = float3(0.f, 1.f, 0.f);// 
    dir = normalize(reflect(dir, N));// normalize(dirOut);//
}

void updateNorm_2(inout float3  exp_point, inout float3  dir, float t, float3 v1, float3 v2, float3 v3) {
    float3 v1v2 = v1 - v2;
    float3 v1v3 = v1 - v3;
    float3 N = normalize(cross(v1v3, v1v2));
    //float3 dirOut = dir - N * 2 * dot(dir, N);
    exp_point = exp_point + dir * (t);
    dir = normalize(reflect(dir, N )); //float3(0.f, 1.f, 0.f); // //normalize(dirOut);//normalize(reflect(dir, N));

}

void ray_march(inout float3 rayOrigin, inout float3 rayDir, inout int hitCount, float4x4 worldMat, float3x3 invWorldMat)
{
    float3 exp_point = rayOrigin;
    float3 dir = rayDir;
    float3 current_block = floor(rayOrigin);
    float3 v1 = float3(0.f);
    float3 v2 = float3(0.f);
    float3 v3 = float3(0.f);
    float3 v4 = float3(0.f);
    ///*Triangle Variables*/
    float t0 = 0.f;
    float t = 0.f;
    float u = 0.f;
    float v = 0.f;
    bool isTriaIntersect = false;
    ///*Line Variables*/
    float4 verticalLine = float4(0.f);
    float4 horizontalLine = float4(0.f);
    float t1 = 0.f;
    float t2 = 0.f;

    bool upper = false;
    bool lower = false;
    int negativeLen = 10;
    while (

       (exp_point.x >= 1.f && exp_point.x <= surfaceSize[1] + 1.f) &&
       (exp_point.z >= 1.f && exp_point.z <= surfaceSize[0] + 1.f) &&
       (exp_point.y >= -1.f && exp_point.y <=  3.f) &&
        (dir.y != 1.f && dir.y != -1.f) &&// (dir.x != 0.f || dir.z != 0.f) &&
        hitCount >= 0 //&& negativeLen >= 0 &&
        //!isTriaIntersect
        )
    {
        t0 = 0.f;
        t = 0.f;

        t1 = 0.f;
        t2 = 0.f;
        verticalLine = float4(0.f);
        horizontalLine = float4(0.f);

        current_block = floor(exp_point);
        fillVertices(current_block.x, current_block.z, v1, v2, v3, v4, worldMat, invWorldMat);
        /*Ray-Triangle Intersection*/
        bool rayTriangleIntersect_1 = rayTriangleIntersect(exp_point, dir, v1, v2, v3, t, u, v);
        bool rayTriangleIntersect_2 = rayTriangleIntersect(exp_point, dir, v4, v3, v2, t0, u, v);
        //if (rayTriangleIntersect_1 && rayTriangleIntersect_2) {
        //    if (t != 0 && t0 != 0 && abs(t) < abs(t0) ) {
        //        updateNorm_1(exp_point, dir, t, v1, v2, v3);
        //        //dir = float3(0.f, 1.f, 0.f);
        //        isTriaIntersect = true;
        //        hitCount--;

        //    }
        //    else if (t != 0 && t0 != 0 && abs(t0) < abs(t)) {
        //        //updateNorm_2(exp_point, dir, t0, v4, v2, v3);
        //        dir = float3(0.f, 1.f, 0.f);
        //        isTriaIntersect = true;
        //        hitCount--;

        //    }
        //}
        if (rayTriangleIntersect_1 ) {
          //  if (t > 0.f) {
                updateNorm_1(exp_point, dir, t, v1, v2, v3);
                isTriaIntersect = true;
                hitCount--;
           // }
        }
        else if (rayTriangleIntersect_2 ) {
           // if (t0 > 0.f) {   
                updateNorm_2(exp_point, dir, t0, v4, v3, v2);
                isTriaIntersect = true;
                hitCount--;

           // }
        }
       // dir = float3(0.f, 1.f, 0.f);
       // hitCount--;

        current_block = floor(exp_point);
        fillVertices(current_block.x, current_block.z, v1, v2, v3, v4, worldMat, invWorldMat);

        ///*Cells Shifting*/
        if (!isTriaIntersect) {
            findLines(exp_point, dir, v1, v2, v3, v4, verticalLine, horizontalLine);
            bool intersection_1 = rayLineIntersect(float2(exp_point.x, exp_point.z), float2(dir.x, dir.z), float2(verticalLine[0], verticalLine[1]), float2(verticalLine[2], verticalLine[3]), t1);
            bool intersection_2 = rayLineIntersect(float2(exp_point.x, exp_point.z), float2(dir.x, dir.z), float2(horizontalLine[0], horizontalLine[1]), float2(horizontalLine[2], horizontalLine[3]), t2);
            if (dir.x < 0.f || dir.z < 0.f) {
                if (intersection_1 && intersection_2) {
                    if (t1 < t2) {
                        exp_point = exp_point + dir * (t1 + 0.01);
                    }
                    else {
                        exp_point = exp_point + dir * (t2 + 0.01);
                    }
                }
                else if (intersection_1) {
                    exp_point = exp_point + dir * (t1 + 0.01);
                }
                else if (intersection_2) {
                    exp_point = exp_point + dir * (t2 + 0.01);
                }
            }
            else {
                if (intersection_1 && intersection_2) {
                    if (t1 < t2) {
                        exp_point = exp_point + dir * (t1 + 0.01);
                    }
                    else {
                        exp_point = exp_point + dir * (t2 + 0.01);
                    }
                }
                else if (intersection_1) {
                    exp_point = exp_point + dir * (t1 + 0.01);
                }
                else if (intersection_2) {
                    exp_point = exp_point + dir * (t2 + 0.01);
                }


            }

        }

        isTriaIntersect = false;



 
       
    }

    rayOrigin = exp_point;
    rayDir = dir;
}



uint pcg_hash(uint input)
{
    uint state = input * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}


float3 RandomGradient3(int sampleNum)
{
    uint seed = sampleNum;// pcg_hash(z ^ pcg_hash(y ^ pcg_hash(x)));
    uint s1 = seed ^ pcg_hash(seed);
    uint s2 = s1 ^ pcg_hash(s1);
    uint s3 = s2 ^ pcg_hash(s2);

    float g1 = (floatConstruct(s1) - 0.5) * 2.0;
    float g2 = (floatConstruct(s2) - 0.5) * 2.0;
    float g3 = (floatConstruct(s3) - 0.5) * 2.0;

    return float3(g1, g2, g3);
}
