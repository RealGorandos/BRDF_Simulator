import Scene.Raster;
import Rendering.Lights.LightHelpers;
import Utils.Sampling.TinyUniformSampleGenerator;
import Utils.Math.MathHelpers;



cbuffer PerFrameCB : register(b0)
{

};



VSOut vsMain(VSIn vIn)
{

    VSOut resVS = defaultVS(vIn);
    return resVS;
}




float4 psMain(VSOut vsOut) : SV_TARGET
{
    
return float4(0.f, 0.f, 0.f, 1.f);
}
