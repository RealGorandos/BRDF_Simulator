/***************************************************************************
 # Copyright (c) 2015-22, NVIDIA CORPORATION. All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions
 # are met:
 #  * Redistributions of source code must retain the above copyright
 #    notice, this list of conditions and the following disclaimer.
 #  * Redistributions in binary form must reproduce the above copyright
 #    notice, this list of conditions and the following disclaimer in the
 #    documentation and/or other materials provided with the distribution.
 #  * Neither the name of NVIDIA CORPORATION nor the names of its
 #    contributors may be used to endorse or promote products derived
 #    from this software without specific prior written permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY
 # EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 # IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 # PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 # CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 # PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 # PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 # OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 # OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **************************************************************************/
#pragma once
#include "Falcor.h"

using namespace Falcor;

class BRDF_Simulator : public IRenderer
{
public:

    struct BRDF_Object {
        Falcor::float3 position;
        std::string materialName;
        Falcor::ShadingModel shadingModel;
    };

    enum class BRDF_Type
    {
        BRDF_Simulation,
        Cook_Torrance,
        GGX
    };

    void onLoad(RenderContext* pRenderContext) override;
    void onFrameRender(RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo) override;
    void onResizeSwapChain(uint32_t width, uint32_t height) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onGuiRender(Gui* pGui) override;

private:
    void loadModel(ResourceFormat fboFormat);
    void loadModelFromFile(const std::filesystem::path& path, ResourceFormat fboFormat);
    void renderSurface();
    void resetCamera();
    void setModelString(double loadTime);
    void setCamController();
    void setEnvMapPipeline();
    void setEnvMapShaderVars();
    void loadOrthoQuad();
    void setOrthoCubeVars();
    void setSceneVars();
    Sampler::SharedPtr mpPointSampler = nullptr;
    Sampler::SharedPtr mpLinearSampler = nullptr;
    Sampler::SharedPtr mpCubePointSampler = nullptr;
    //Main scene PIPELINE
    Scene::SharedPtr mpScene;
    GraphicsProgram::SharedPtr mpProgram = nullptr;
    GraphicsVars::SharedPtr mpProgramVars = nullptr;
    GraphicsState::SharedPtr mpGraphicsState = nullptr;
    //Model PIPELINE
    Scene::SharedPtr mpModelScene;
    GraphicsProgram::SharedPtr mpModelProgram = nullptr;
    GraphicsVars::SharedPtr mpModelProgramVars = nullptr;
    GraphicsState::SharedPtr mpModelGraphicsState = nullptr;
    //CubeBox scene PIPELINE
    Scene::SharedPtr mpCubeScene;
    GraphicsProgram::SharedPtr mpCubeProgram = nullptr;
    GraphicsVars::SharedPtr mpCubeProgramVars = nullptr;
    GraphicsState::SharedPtr mpCubeGraphicsState = nullptr;
    RasterizerState::SharedPtr mpRsState = nullptr;
    RasterizerState::CullMode mCubeCullMode = RasterizerState::CullMode::None;
    Fbo::SharedPtr mpFbo;
    Texture::SharedPtr pTex;
    Sampler::SharedPtr mpSampler;


    //Orthographic Camera simulation PIPELINE
    bool mUseTriLinearFiltering = true;
    Sampler::SharedPtr mpDebuggingQuadPointSampler = nullptr;
    Sampler::SharedPtr mpDebuggingQuadLinearSampler = nullptr;

    GraphicsProgram::SharedPtr mpDebuggingQuadProgram = nullptr;
    GraphicsVars::SharedPtr mpDebuggingQuadProgramVars = nullptr;
    GraphicsState::SharedPtr mpDebuggingQuadGraphicsState = nullptr;

    bool mpDebuggingQuadDrawWireframe = false;

    SceneBuilder::SharedPtr mpDebuggingQuadSceneBuilder;
    Scene::SharedPtr mpDebuggingQuadScene;
    RasterizerState::SharedPtr mpDebuggingQuadWireframeRS = nullptr;

    DepthStencilState::SharedPtr mpDebuggingQuadNoDepthDS = nullptr;
    DepthStencilState::SharedPtr mpDebuggingQuadDepthTestDS = nullptr;
    float cameraSize = 0.110000f;
    //////
    bool mUseOriginalTangents = false;
    bool mDontMergeMaterials = false;

    
   
    Scene::CameraControllerType mCameraType = Scene::CameraControllerType::Orbiter;

    
    BRDF_Simulator::BRDF_Type mBRDFType = BRDF_Simulator::BRDF_Type::BRDF_Simulation;
    
    SceneBuilder::SharedPtr mSceneBuilder ;

    //WIREFRAME VARIABLEs

    RasterizerState::CullMode mCullMode = RasterizerState::CullMode::Back;
    bool mDrawWireframe = false;

    DepthStencilState::SharedPtr mpNoDepthDS = nullptr;
    DepthStencilState::SharedPtr mpDepthTestDS = nullptr;

    Falcor::int2 planSizeTemp = Falcor::int2(1);
    Falcor::int2 planSize = Falcor::int2(1);
    float orthCamWidth = 2;
    float orthCamHeight = 1;
    float roughness = 0.f;
    int jitterNum = 3;
    int bounces = 0;
    int jitterInternal = 0;
    int bouncesInternal = 0;
    float3 camCurrPos = float3(0.f);
    Falcor::float3 shapePosition = Falcor::float3(0);

    float  orthoLeft;
    float  orthoRight;
    float  orthoTop;
    float  orthoBottom;
    float  pixelsNum;
    //Orthographic Camera Data
    Falcor::rmcv::mat4 storeProjMat = Falcor::rmcv::mat4(1.f);
    bool mOrthoCam = false;

    std::string mModelString;
    bool showDebuggingQuad = false;

    bool debuggingQuad = false;

    bool BRDF_Simulation = false;
    bool clearTexture = false;
    float3 rotateQuad = float3(3.15f, 0.f, 0.f);
    float3 orthoCamPostion = float3(0.f ,-23.500000f, 0.f);

    bool mMicrofacetes = true;
    bool mObjectSimulation = false;
};
