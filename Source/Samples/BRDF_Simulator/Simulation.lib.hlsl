#define PI 3.14159265358979323846264338327950288
#define kEpsilon 0.000001

/*________Helper Methods for finding Quads and Lines_______________________*/

void fillVertices(const int x, const int y, inout float3 v1, inout float3 v2, inout float3 v3, inout float3 v4) {
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
    //if (( det < kEpsilon)) return false;

    float invDet = 1.0 / det;

    float3 tvec = orig - v0;
    u = dot(tvec, pvec) *invDet;
    if (u < 0.f || u > 1.f) return false;

    float3 qvec = cross(tvec, v0v1);
    v = dot(dir, qvec) *invDet;
    if (v < 0.f || v + u> 1.f) return false;

    t = dot(v0v2, qvec) * invDet;

    if (t <= 0) return false;
   
    


    return true;

}
/*Ray Line intersection Function*/
bool rayLineIntersect(const float2 rayOrigin, const float2 rayDirection, const float2 lineStart, const float2 lineEnd, inout float t) {


    float2 lineDir = lineEnd - lineStart;

    float d = rayDirection.x * lineDir.y - rayDirection.y * lineDir.x;


    if (d == 0.f) return false;

    float u = ((lineStart.x - rayOrigin.x) * rayDirection.y - (lineStart.y - rayOrigin.y) * rayDirection.x) / d;
    t = ((lineStart.x - rayOrigin.x) * lineDir.y - (lineStart.y - rayOrigin.y) * lineDir.x) / d;

    if ((u < 0.f || u > 1.f) || (t <= 0.f)) return false;

    return true;
}


/*_________________________________Ray marching function (For the simulation)___________________*/

/*Updates direction*/
void updateDir(inout float3  exp_point, inout float3  dir, float t, float3 v1, float3 v2, float3 v3) {
    float3 v1v2 = (v2 - v1);
    float3 v1v3 = (v3 - v1);
    float3 N = normalize(cross(v1v3, v1v2));

    exp_point = exp_point + dir * (t);//  -dir * 0.01f;


    dir = normalize(reflect(dir, N)); 


}
/*Checks if the point is still in the Surface*/
bool isPointInCube(float3 exp_point, float3 quadDeter, int2 surfaceSize) {

    return (exp_point.x >= 1.f && exp_point.x <= surfaceSize[1] + 1.f) &&
           (exp_point.z >= 1.f && exp_point.z <= surfaceSize[0] + 1.f) &&
           (exp_point.y >= 0.f && exp_point.y <= 5.f) &&

            (quadDeter.x >= 1.f && quadDeter.x <= surfaceSize[1] + 1.f) &&
            (quadDeter.z >= 1.f && quadDeter.z <= surfaceSize[0] + 1.f) &&
            (quadDeter.y >= 0.f && quadDeter.y <= 5.f);

}

/*___Checks if there is an intersection between a point and a triangle___*/
bool isTriangleIntersection(inout float3 exp_point,inout float3 dir, float3 v1, float3 v2, float3 v3, float3 v4, inout int hits, inout bool upper, inout bool lower) {
    ///*Triangle Variables*/
    float t0 = 0.f;
    float t = 0.f;
    float u = 0.f;
    float v = 0.f;
    bool isTriaIntersect = false;


    //Checking if there is a Ray-Triangle Intersection in the current block
    bool rayTriangleIntersect_Upper = rayTriangleIntersect(exp_point, dir, v1, v2, v3, t, u, v);
    bool rayTriangleIntersect_Lower = rayTriangleIntersect(exp_point, dir, v4, v3, v2, t0, u, v);


    //Checking Upper Triangle
    if (rayTriangleIntersect_Upper && !upper) {
        //dir = float3(0.f, 1.f, 0.f);
        updateDir(exp_point, dir, t, v1, v2, v3);
        isTriaIntersect = true;
        upper = true;
        lower = false;
        hits--;
    }
    ////Checking Lower Triangle
    else if (rayTriangleIntersect_Lower && !lower) {
        //dir = float3(0.f, 1.f, 0.f);
        updateDir(exp_point, dir, t0 , v4, v3, v2);
        isTriaIntersect = true;
        upper = false;
        lower = true;
        hits--;
    }

    return isTriaIntersect;
}

void auxIsTriangleIntersection(inout float3 exp_point, inout float3 dir, float3 v1, float3 v2, float3 v3, float3 v4, inout int hits, inout bool upper, inout bool lower) {

    ///*Triangle Variables*/
    float t0 = 0.f;
    float t = 0.f;
    float u = 0.f;
    float v = 0.f;
    bool isTriaIntersect = false;


    //Checking if there is a Ray-Triangle Intersection in the current block
    bool rayTriangleIntersect_Upper = rayTriangleIntersect(exp_point, dir, v1, v2, v3, t, u, v);
    bool rayTriangleIntersect_Lower = rayTriangleIntersect(exp_point, dir, v4, v3, v2, t0, u, v);

    if (rayTriangleIntersect_Upper && rayTriangleIntersect_Lower) {
        if (t0 < t) {
            updateDir(exp_point, dir, t, v1, v2, v3);
            upper = true;
            lower = false;
            hits--;
        }
        else if (t < t0) {
            updateDir(exp_point, dir, t0, v4, v3, v2);
            upper = false;
            lower = true;
            hits--;
        }

    }
    //Checking Upper Triangle
    else if (rayTriangleIntersect_Upper) {
        //dir = float3(0.f, 1.f, 0.f);
        updateDir(exp_point, dir, t, v1, v2, v3);
        isTriaIntersect = true;
        upper = true;
        lower = false;
        hits--;
    }
    ////Checking Lower Triangle
    else if (rayTriangleIntersect_Lower) {
        //dir = float3(0.f, 1.f, 0.f);
        updateDir(exp_point, dir, t0 , v4, v3, v2);
        isTriaIntersect = true;
        upper = false;
        lower = true;
        hits--;
    }

}

/*___Update the cell___*/
bool updateCell(inout float3 exp_point, inout float3 quadDeter, inout  float3 dir,float3 v1, float3 v2, float3 v3, float3 v4 ) {
    //Line Variables
    float4 verticalLine = float4(0.f);
    float4 horizontalLine = float4(0.f);
    float t1 = 0.f;
    float t2 = 0.f;

    //Filling the variables
    findLines(exp_point, dir, v1, v2, v3, v4, verticalLine, horizontalLine);

    //Checking if there is a ray-line intersection
    bool intersection_1 = rayLineIntersect(float2(exp_point.x, exp_point.z), float2(dir.x, dir.z), float2(verticalLine[0], verticalLine[1]), float2(verticalLine[2], verticalLine[3]), t1);
    bool intersection_2 = rayLineIntersect(float2(exp_point.x, exp_point.z), float2(dir.x, dir.z), float2(horizontalLine[0], horizontalLine[1]), float2(horizontalLine[2], horizontalLine[3]), t2);

    //Updating position if there is an intersection
    if (intersection_1 && intersection_2) {
        if (t1 < t2) {
            quadDeter = exp_point + dir * (t1) + dir * (0.01f);
            exp_point = exp_point + dir * (t1);

        }
        else {
            quadDeter = exp_point + dir * (t2) + dir * (0.01f);
            exp_point = exp_point + dir * (t2);

        }
    }
    else if (intersection_1) {
        quadDeter = exp_point + dir * (t1 ) + dir * (0.01f);
        exp_point = exp_point + dir * (t1 );
    }
    else if (intersection_2) {
        quadDeter = exp_point + dir * (t2) + dir * (0.01f);
        exp_point = exp_point + dir * (t2 );
    }
    else {
        return false;
    }

    return true;
}


/*March the ray along the surface*/
bool ray_march(inout float3 rayOrigin, inout float3 rayDir, inout int hitCount, bool upper, bool lower)
{
    float3 exp_point = rayOrigin;
    float3 quadDeter = rayOrigin;
    float3 dir = rayDir;
    float3 current_block = floor(rayOrigin);
    float3 v1 = float3(0.f);
    float3 v2 = float3(0.f);
    float3 v3 = float3(0.f);
    float3 v4 = float3(0.f);

    bool isTriaIntersect = false;

    int hits = hitCount;

    bool upperIn = upper;
    bool lowerIn = lower;
    int intersectionCnt = 5;
    while (hits >= 0 &&
        isPointInCube(exp_point, quadDeter, surfaceSize) &&
        intersectionCnt >= 0
        )
    {
            //Finding the Current Block
            current_block = floor(quadDeter);
            fillVertices(current_block.x, current_block.z, v1, v2, v3, v4);

            ////Checking if there is a triangle intersection
            isTriaIntersect = isTriangleIntersection(exp_point, dir, v1, v2, v3, v4, hits, upperIn, lowerIn);
            intersectionCnt--;
            if (!isTriaIntersect) {
                 upperIn = false;
                 lowerIn = false;
                 if(updateCell(exp_point, quadDeter, dir, v1, v2, v3, v4)) {
                     intersectionCnt = 5;
                     //Finding the Current Block
                     current_block = floor(quadDeter);
                     fillVertices(current_block.x, current_block.z, v1, v2, v3, v4);

                     ////Checking if there is a triangle intersection
                     auxIsTriangleIntersection(exp_point, dir, v1, v2, v3, v4, hits, upperIn, lowerIn);
                 }
            }
    }

    rayOrigin = exp_point;
    rayDir = dir;
    hitCount = hits;
    bool defected = dot(float3(0.f, -1.f, 0.f), rayDir) > 0.3f;
    return !isPointInCube(rayOrigin, quadDeter, surfaceSize) && hitCount >= 0 && !defected  ;
}
