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
#include <vector>
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
        Cook_Torrance
    };

    void onLoad(RenderContext* pRenderContext) override;
    void onFrameRender(RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo) override;
    void onResizeSwapChain(uint32_t width, uint32_t height) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onGuiRender(Gui* pGui) override;

private:
    //Loading model from a file
    void loadModel(ResourceFormat fboFormat);
    void loadModelFromFile(const std::filesystem::path& path, ResourceFormat fboFormat);
    void envMapConvert(Falcor::EnvMap& envMap, int currLayer);
    //Render the microfacts surface
    void renderSurface();

    //Camera functions
    void resetCamera();
    void setCamController();
    void setModelString(double loadTime);

    //Render the enviroment map and setup the pipeline
    void setEnvMapPipeline();

    //Passing variables to the pipelines
    void setEnvMapShaderVars();
    void setModelVars();
    void setOrthoVisualizorVars();
    void setSceneVars();

    //load the camera visualization Quad
    void loadOrthoVisualizor(int currLayer);


    //Surface pipeline gui
    void loadSurfaceGUI(Gui::Window& w);
    void loadModelGUI(Gui::Window& w);

    //Rasterize pipelines
    void rasterizeSurfaceView(RenderContext* pRenderContext);
    void rasterizeModelView(RenderContext* pRenderContext);

    //Jitter camera
    void jitterCamera();

    //Continous Simulation
    void continousSimulation();

    //Update EnvMap texture function
    void  updateEnvMapTexture(bool clear, bool update, bool get, int currLayer);

    void setEnvMapModelShaderVars();

    Sampler::SharedPtr mpPointSampler = nullptr;
    Sampler::SharedPtr mpLinearSampler = nullptr;
    Sampler::SharedPtr mpCubePointSampler = nullptr;
    DepthStencilState::SharedPtr mpNoDepthDS = nullptr;
    DepthStencilState::SharedPtr mpDepthTestDS = nullptr;
    std::string mModelString;

    //Main scene PIPELINE
    Scene::SharedPtr mpScene;
    GraphicsProgram::SharedPtr mpProgram = nullptr;
    GraphicsVars::SharedPtr mpProgramVars = nullptr;
    GraphicsState::SharedPtr mpGraphicsState = nullptr;
    SceneBuilder::SharedPtr mSceneBuilder;

    //Model PIPELINE
    Scene::SharedPtr mpModelScene;
    GraphicsProgram::SharedPtr mpModelProgram = nullptr;
    GraphicsVars::SharedPtr mpModelProgramVars = nullptr;
    GraphicsState::SharedPtr mpModelGraphicsState = nullptr;
    bool mUseTriLinearFiltering = true;
    bool mUseOriginalTangents = false;
    bool mDontMergeMaterials = false;

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
    GraphicsProgram::SharedPtr mpDebuggingQuadProgram = nullptr;
    GraphicsVars::SharedPtr mpDebuggingQuadProgramVars = nullptr;
    GraphicsState::SharedPtr mpDebuggingQuadGraphicsState = nullptr;
    SceneBuilder::SharedPtr mpDebuggingQuadSceneBuilder;
    Scene::SharedPtr mpDebuggingQuadScene;
    RasterizerState::SharedPtr mpDebuggingQuadWireframeRS = nullptr;
    DepthStencilState::SharedPtr mpDebuggingQuadNoDepthDS = nullptr;
    DepthStencilState::SharedPtr mpDebuggingQuadDepthTestDS = nullptr;

    
   //Dropdown lists
    Scene::CameraControllerType mCameraType = Scene::CameraControllerType::Orbiter;
    BRDF_Simulator::BRDF_Type mBRDFType = BRDF_Simulator::BRDF_Type::Cook_Torrance;
    

    //Surface variables
    Falcor::int2 planSize = Falcor::int2(60,60); //Surface Of Microfacets size
    float roughness = 0.f;

    //Ortho Camera variables
    float orthCamWidth = 2;
    float orthCamHeight = 1;
    bool mOrthoCam = false;

    //Simulation Variables
    int jitterNum = 3;
    int bounces = 0;
    int jitterInternal = 2;
    int bouncesInternal = 2;
    bool switchBool = false;

    bool contSwitchBool = false;
    //Buttons variables
    bool BRDF_Simulation = false;
    bool clearTexture = false;
    bool continous_simulation = false;

    //OrthoQuad Visualization variables
    float3 up = float3(0.f, 1.f, 0.f);
    float3 rotateQuad = float3(0.f, 0.f, 90.f);
    float3 orthoCamPostion = float3(0.f, 0.f, 0.f);
    float3 QuadLookAt = float3(1.5f, 0.f, 1.5f);
    int degOfRotation = 15;
    int maxLayer = 10;
    float3 cameraPos = float3(0.f);

    //Layers variables
    int currLayer = 1;
    int currLayerTemp = 1;
    int currLayerInternal = 1;
    //Falcor::Texture::SharedPtr var = Falcor::Texture::create2D(150, 150, ResourceFormat::R32Uint, 1, 1, nullptr, ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess);
    std::vector<Falcor::EnvMap::SharedPtr> textureVect;
    //Model view switching variables
    bool mMicrofacetes = true;
    bool mObjectSimulation = false;

    //BRDF method boolean
    bool isCookTorrence = false;

    //Our BRDF boolean
    bool isSimulation = false;

    //Run brdf simulation
    bool runSimulation = false;

    
    //Cook-torrence variables
    float metallic = 0.f;
    float mRoughness = 0.f;
    float3 mAlbedo = float3(0.24f, 0.24f ,0.24f);
    float ao = float(1.f);



    int layerCnt = 2;
    int timer = 300;
};
