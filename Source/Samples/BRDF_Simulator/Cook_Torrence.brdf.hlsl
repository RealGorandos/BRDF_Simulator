

//float3 schlick_frasnel(float3 N, float3 V, float3 F0)
//{
//    return F0 + (1.0 - F0) * pow(1.0 - dot(N, V), 5.0);
//}
//
//float beckmann_distribution(float3 N, float3 H, float a) {
//    float nh = dot(N, H);
//    float nom = exp((nh * nh - 1.) / (a * a * nh * nh));
//    float den = 4. * a * a * nh * nh * nh * nh;
//
//    return nom / max(den, 1e-4);
//}
//
//float cook_torrance_geometry(float3 N, float3 V, float3 L) {
//    L = reflect(-L, N);
//    float3 h = normalize(V + L);
//    float nh = dot(N, h);
//    float nv = dot(N, V);
//    float vh = dot(V, h);
//    float nl = dot(N, L);
//    return min(1., min((2. * nh * nv) / vh, (2. * nh * nl) / vh));
//}
//
//float3 cook_torrance_origin_brdf(float3 L, float3 V, float3 N) {
//
//   // float a = iParameter0 * iParameter0;
//    float3 H = normalize(L + V);
//    float3 PLASTIC_SPECULAR_COLOR = float3(0.25, 0.25, 0.25);
//    float3 F = schlick_frasnel(N, V, PLASTIC_SPECULAR_COLOR);
//   // float D = beckmann_distribution(N, H, a);
//    float G = cook_torrance_geometry(N, V, L);
//    float3 specular = (F * D * G) / (4. * dot(L, N) * dot(V, N));
//    //vec3 diffuse = vec3(kd)*1./PI; 
//    return specular;
//}


//float3 render(float3 L, float3 N, float3 V, float2 textureCord, float3x3 worldToLocal) {
//    float3 col = float3(1.f, 1.f, 1.f);//texture(skybox, L).rgb;
//    return col * cook_torrance_origin_brdf(L, V, N);
//}
