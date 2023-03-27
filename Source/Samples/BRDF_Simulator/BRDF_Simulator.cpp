#include "BRDF_Simulator.h"
#include "Utils/UI/TextRenderer.h"
#include "Utils/Math/FalcorMath.h"


void BRDF_Simulator::setModelString(double loadTime)
{
    FALCOR_ASSERT(mpScene != nullptr);

    mModelString = "Loading took " + std::to_string(loadTime) + " seconds.\n";
    mModelString += std::to_string(mpScene->getMeshCount()) + " meshes, ";
    mModelString += std::to_string(mpScene->getGeometryInstanceCount()) + " instances, ";
    mModelString += std::to_string(mpScene->getMaterialCount()) + " materials, ";
}


void BRDF_Simulator::setEnvMapPipeline() {
    

    mpCubeScene = Scene::create("cube.obj");

    mpCubeProgram = GraphicsProgram::createFromFile("Samples/BRDF_Simulator/EnvMap.3d.hlsl", "vsMain", "psMain");
    mpCubeProgram->addDefines(mpCubeScene->getSceneDefines());
    mpCubeProgramVars = GraphicsVars::create(mpCubeProgram->getReflector());
    mpFbo = Fbo::create();

    // Create state
    mpCubeGraphicsState = GraphicsState::create();
    BlendState::Desc blendDesc;
    for (uint32_t i = 1; i < Fbo::getMaxColorTargetCount(); i++) blendDesc.setRenderTargetWriteMask(i, false, false, false, false);
    blendDesc.setIndependentBlend(true);
    mpCubeGraphicsState->setBlendState(BlendState::create(blendDesc));

    // Create the rasterizer state
    RasterizerState::Desc rastDesc;
    rastDesc.setCullMode(RasterizerState::CullMode::None).setDepthClamp(true);
    mpRsState = RasterizerState::create(rastDesc);

    DepthStencilState::Desc dsDesc;
    dsDesc.setDepthWriteMask(false).setDepthFunc(DepthStencilState::Func::LessEqual);
    mpCubeGraphicsState->setDepthStencilState(DepthStencilState::create(dsDesc));
    mpCubeGraphicsState->setProgram(mpCubeProgram);

    Sampler::Desc samplerDesc;
    samplerDesc.setFilterMode(Sampler::Filter::Point, Sampler::Filter::Point, Sampler::Filter::Point);
    mpCubePointSampler = Sampler::create(samplerDesc);
   // std::filesystem::path path;

    auto pTex = Texture::create2D(100, 100, ResourceFormat::R32Uint, 1, 1, nullptr, ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess);
    pTex->setName("Enviroment Map");
    
    mpCubeScene->setEnvMap(EnvMap::create(pTex));
    mpCubeScene->getMaterialSystem()->setDefaultTextureSampler(mpCubePointSampler);

}

void BRDF_Simulator::setEnvMapShaderVars() {

    const auto& pEnvMap = mpCubeScene->getEnvMap();
    mpCubeProgram->addDefine("_USE_ENV_MAP", pEnvMap ? "1" : "0");
    if (pEnvMap) {
        mpCubeProgramVars["PerFrameCB"]["tex2D_uav"].setTexture(pEnvMap->getEnvMap());
        mpCubeProgramVars["PerFrameCB"]["gSamples"] = sampleNum;
            //setUav(pEnvMap->getEnvMap()->getUAV(0));
        mpCubeProgramVars["PerFrameCB"]["envSampler"].setSampler(pEnvMap->getEnvSampler());
        //pEnvMap->setShaderData(mpCubeProgramVars["PerFrameCB"]["gEnvMap"]);
    }

    rmcv::mat4 world = rmcv::translate(mpScene->getCamera()->getPosition());

    mpCubeProgramVars["PerFrameCB"]["gWorld"] = world;
    mpCubeProgramVars["PerFrameCB"]["gScale"] = 1.f;
    mpCubeProgramVars["PerFrameCB"]["gViewMat"] = mpScene->getCamera()->getViewMatrix();
    mpCubeProgramVars["PerFrameCB"]["gProjMat"] = mpCubeScene->getCamera()->getProjMatrix();

}

void BRDF_Simulator::renderSurface() {

    CpuTimer timer;
    timer.update();

    mSceneBuilder = SceneBuilder::create(SceneBuilder::Flags::None);
    SceneBuilder::Node N;
    Falcor::StandardMaterial::SharedPtr Material = StandardMaterial::create("Surface Material", ShadingModel::MetalRough);
    for (int row = 0; row < planSize[0]; row++) {
        for (int col = 0; col < planSize[1]; col++) {
            N.transform[0][3] = float(col);
            N.transform[2][3] = float(row);
            mSceneBuilder->addMeshInstance(mSceneBuilder->addNode(N), mSceneBuilder->addTriangleMesh(TriangleMesh::createQuad(float2(1.f)), Material));
        }
    }

    mpScene = mSceneBuilder->getScene();

    {
        Program::Desc desc;
        desc.addShaderModules(mpScene->getShaderModules());
        desc.addShaderLibrary("Samples/BRDF_Simulator/BRDF_Simulator.3d.hlsl").vsEntry("vsMain").gsEntry("gsMain").psEntry("psMain");
        desc.addTypeConformances(mpScene->getTypeConformances());

        mpProgram = GraphicsProgram::create(desc, mpScene->getSceneDefines());
    }

    mpProgramVars = GraphicsVars::create(mpProgram->getReflector());
    mpGraphicsState->setProgram(mpProgram);

    setCamController();
    timer.update();
    setModelString(timer.delta());
}


void BRDF_Simulator::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "BRDF Simulator Demo", { 400, 300 }, { 0, 100 });

    {
        auto gridSettings = w.group("Surface Settings");
        gridSettings.var("Surface Size", planSizeTemp, 10);
        if (gridSettings.button("Update Surface")) {
            planSize = planSizeTemp;
            renderSurface();
        }
        gridSettings.var("Roughness", roughness, 1,10);
    }


    {
        auto loadGroup = w.group("Load Options");
        loadGroup.checkbox("Use Original Tangents", mUseOriginalTangents);
        loadGroup.tooltip("If this is unchecked, we will ignore the tangents that were loaded from the model and calculate them internally. Check this box if you'd like to use the original tangents");
        loadGroup.checkbox("Don't Merge Materials", mDontMergeMaterials);
        loadGroup.tooltip("Don't merge materials that have the same properties. Use this option to preserve the original material names.");
    }
    w.var("Samples", sampleNum, 3);
    w.separator();




    Gui::DropdownList cameraDropdown;
    cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::FirstPerson, "First-Person" });
    cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::Orbiter, "Orbiter" });
    cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::SixDOF, "6-DoF" });

    if (w.dropdown("Camera Type", cameraDropdown, (uint32_t&)mCameraType)) setCamController();

    if (w.checkbox("Orthographic View", mOrthoCam)) { resetCamera(); }
    w.separator();
    startSimulation = w.button("Start Simulation");
    clearTexture = w.button("Clear Texture");
}

void BRDF_Simulator::onLoad(RenderContext* pRenderContext)
{
    
    mpGraphicsState = GraphicsState::create();
    // Create rasterizer state
    RasterizerState::Desc wireframeDesc;
    wireframeDesc.setFillMode(RasterizerState::FillMode::Wireframe);
    wireframeDesc.setCullMode(RasterizerState::CullMode::None);
    mpWireframeRS = RasterizerState::create(wireframeDesc);

    // Depth test
    DepthStencilState::Desc dsDesc;
    dsDesc.setDepthEnabled(false);
    mpNoDepthDS = DepthStencilState::create(dsDesc);
    dsDesc.setDepthFunc(ComparisonFunc::Less).setDepthEnabled(true);
    mpDepthTestDS = DepthStencilState::create(dsDesc);

    Sampler::Desc samplerDesc;
    samplerDesc.setFilterMode(Sampler::Filter::Point, Sampler::Filter::Point, Sampler::Filter::Point);
    mpPointSampler = Sampler::create(samplerDesc);
    samplerDesc.setFilterMode(Sampler::Filter::Point, Sampler::Filter::Point, Sampler::Filter::Point);
    mpLinearSampler = Sampler::create(samplerDesc);

    resetCamera();
    setEnvMapPipeline();
    renderSurface();

    mpScene->getMaterialSystem()->setDefaultTextureSampler(mpPointSampler);
    //FIX
    orthoLeft = -40.0f * mpScene->getCamera()->getAspectRatio();
    orthoRight = 40.0f * mpScene->getCamera()->getAspectRatio();
    orthoTop = -40.0f;
    orthoBottom = 40.0f;
    pixelsNum = (orthoRight - orthoLeft) * (orthoBottom - orthoTop);
}



void BRDF_Simulator::onFrameRender(RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo)
{

    const float4 clearColor(0.4f, 0.4f, 0.4f, 1);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);
    mpGraphicsState->setFbo(pTargetFbo);
    if (clearTexture) {
        clearTexture = !clearTexture;
        setEnvMapPipeline();
    }

    mpCubeGraphicsState->setFbo(pTargetFbo);

    setEnvMapShaderVars();
   
    if (mpScene)
    {
        mpScene->update(pRenderContext, gpFramework->getGlobalClock().getTime());

        // Set render state

        
        {
            mpGraphicsState->setDepthStencilState(mpDepthTestDS);
            mpProgramVars["PerFrameCB"]["gConstColor"] = false;
            mpProgramVars["PerFrameCB"]["simulate"] = this->startSimulation;
            mpProgramVars["PerFrameCB"]["roughness"] = roughness;
            const auto& pEnvMap = mpCubeScene->getEnvMap();
            if (pEnvMap) {
                mpProgramVars["PerFrameCB"]["tex2D_uav"].setUav(pEnvMap->getEnvMap()->getUAV(0));
                //setUav(pEnvMap->getEnvMap()->getUAV(0));
                mpProgramVars["PerFrameCB"]["envSampler"].setSampler(pEnvMap->getEnvSampler());

                mpProgramVars["PerFrameCB"]["samples"] = sampleNum;
                std::cout <<  sampleNum << std::endl;
            }

            
            mpScene->rasterize(pRenderContext, mpGraphicsState.get(), mpProgramVars.get(), RasterizerState::CullMode::None);
            mpCubeScene->rasterize(pRenderContext, mpCubeGraphicsState.get(), mpCubeProgramVars.get(), mpRsState, mpRsState);
            
        }
    }


    if (mOrthoCam) {
        mpScene->getCamera()->setProjectionMatrix(Falcor::rmcv::ortho(-40.0f * mpScene->getCamera()->getAspectRatio(), 40.0f * mpScene->getCamera()->getAspectRatio(), -40.0f , 40.0f, mpScene->getCamera()->getNearPlane(), mpScene->getCamera()->getFarPlane()));
    }
    else {
        mpScene->getCamera()->setProjectionMatrix(Falcor::rmcv::perspective(Falcor::focalLengthToFovY(mpScene->getCamera()->getFocalLength(), mpScene->getCamera()->getFrameHeight()), mpScene->getCamera()->getFrameWidth()/ mpScene->getCamera()->getFrameHeight(), mpScene->getCamera()->getNearPlane(), mpScene->getCamera()->getFarPlane()));

    }
    TextRenderer::render(pRenderContext, mModelString, pTargetFbo, float2(10, 30));

    // change the simulation boolean to false.
    this->startSimulation = false;
}

bool BRDF_Simulator::onKeyEvent(const KeyboardEvent& keyEvent)
{
    if (mpScene && mpScene->onKeyEvent(keyEvent)) return true;

    if ((keyEvent.type == KeyboardEvent::Type::KeyPressed) && (keyEvent.key == Input::Key::R))
    {
        resetCamera();
        return true;
    }
    return false;
}

bool BRDF_Simulator::onMouseEvent(const MouseEvent& mouseEvent)
{
    return mpScene ? mpScene->onMouseEvent(mouseEvent) : false;
}

void BRDF_Simulator::onResizeSwapChain(uint32_t width, uint32_t height)
{
    float h = (float)height;
    float w = (float)width;
}

void BRDF_Simulator::setCamController()
{
    if (mpScene) {mpScene->setCameraController(mCameraType);}

}



void BRDF_Simulator::resetCamera()
{
    if (mpScene)
    {
        mpScene->resetCamera(true);
        setCamController();
    }
}

int main(int argc, char** argv)
{
    BRDF_Simulator::UniquePtr pRenderer = std::make_unique<BRDF_Simulator>();

    SampleConfig config;
    config.windowDesc.title = "BRDF Simulator Demo";
    config.windowDesc.resizableWindow = true;

    Sample::run(config, pRenderer);
    return 0;
}
