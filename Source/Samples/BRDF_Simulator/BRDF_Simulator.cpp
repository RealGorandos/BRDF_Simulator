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


void BRDF_Simulator::loadOrthoQuad() {


    mpOrthoCubeSceneBuilder = SceneBuilder::create(SceneBuilder::Flags::None);
    SceneBuilder::Node N;
    Falcor::StandardMaterial::SharedPtr Material = StandardMaterial::create("Surface Material", ShadingModel::MetalRough);
    N.transform[0][3] = float(0.f);
    N.transform[1][3] = float(0.f);
    N.transform[2][3] = float(0.f);
    Falcor::TriangleMesh::SharedPtr quadTemp = TriangleMesh::createQuad(float2(10.f));
    

    quadTemp->addVertex( float3(-(0.5f * 10.f), planSize[0], -(0.5f * 10.f)), float3(0.f, 0.f, 0.f), float2( 0.f, 0.f ));
    quadTemp->addVertex({ (0.5f * 10.f), planSize[0], -(0.5f * 10.f) }, float3(0.f, 0.f, 0.f), { 1.f, 0.f });
    quadTemp->addVertex({ -(0.5f * 10.f), planSize[0],  (0.5f * 10.f) }, float3(0.f, 0.f, 0.f), { 0.f, 1.f });
    quadTemp->addVertex({ (0.5f * 10.f), planSize[0],  (0.5f * 10.f) }, float3(0.f, 0.f, 0.f), { 1.f, 1.f });

    Falcor::TriangleMesh::IndexList indices = quadTemp->getIndices();

    indices.push_back(4);
    indices.push_back(0);
    indices.push_back(0);

    indices.push_back(5);
    indices.push_back(1);
    indices.push_back(1);

    indices.push_back(6);
    indices.push_back(2);
    indices.push_back(2);

    indices.push_back(7);
    indices.push_back(3);
    indices.push_back(3);

    quadTemp->setIndices(indices);
    
    mpOrthoCubeSceneBuilder->addMeshInstance(mpOrthoCubeSceneBuilder->addNode(N), mpOrthoCubeSceneBuilder->addTriangleMesh(quadTemp, Material));


    mpOrthoCubeScene = mpOrthoCubeSceneBuilder->getScene();

    {
        Program::Desc desc;
        desc.addShaderModules(mpOrthoCubeScene->getShaderModules());
        desc.addShaderLibrary("Samples/BRDF_Simulator/OrthoCam.3d.hlsl").vsEntry("vsMain").gsEntry("gsMain").psEntry("psMain");
        desc.addTypeConformances(mpOrthoCubeScene->getTypeConformances());

        mpOrthoCubeProgram = GraphicsProgram::create(desc, mpOrthoCubeScene->getSceneDefines());
    }

    mpOrthoCubeProgramVars = GraphicsVars::create(mpOrthoCubeProgram->getReflector());


   // mpOrthoCubeGraphicsState = GraphicsState::create();

    // Create rasterizer state
    RasterizerState::Desc orthoCubewireframeDesc;
    orthoCubewireframeDesc.setFillMode(RasterizerState::FillMode::Wireframe);
    orthoCubewireframeDesc.setCullMode(RasterizerState::CullMode::None);//.setDepthClamp(true);
    orthoCubewireframeDesc.setFrontCounterCW(false);
    mpOrthoCubeWireframeRS = RasterizerState::create(orthoCubewireframeDesc);

    // Depth test
    mpOrthoCubeGraphicsState->setProgram(mpOrthoCubeProgram);
    setCamController();
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

void BRDF_Simulator::setOrthoCubeVars() {
   // mpOrthoCubeScene->setCamera(mpScene->getCamera());
    rmcv::mat4 world = rmcv::translate(mpOrthoCubeScene->getCamera()->getPosition());
    //mpOrthoCubeScene->setCamera(mpScene->getCamera());
    mpOrthoCubeProgramVars["PerFrameCB"]["gWorld"] = world;
    //mpOrthoCubeProgramVars["PerFrameCB"]["gScale"] = 1.f;
    mpOrthoCubeProgramVars["PerFrameCB"]["gViewMat"] = mpScene->getCamera()->getViewMatrix();
    mpOrthoCubeProgramVars["PerFrameCB"]["gProjMat"] = mpScene->getCamera()->getProjMatrix();
    mpOrthoCubeProgramVars["PerFrameCB"]["gConstColor"] = false;

    mpOrthoCubeProgramVars["PerFrameCB"]["rotation"] = rotateQuad;
    mpOrthoCubeProgramVars["PerFrameCB"]["cameraSize"] = cameraSize;
    mpOrthoCubeProgramVars["PerFrameCB"]["camPosition"] = orthoCamPostion;
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
    std::cout << "------------------------x: " + std::to_string(planSize[1]) << std::endl;
    std::cout << "------------------------y: " + std::to_string(planSize[0]) << std::endl;

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
        gridSettings.var("Surface Size", planSizeTemp, 1);
        if (gridSettings.button("Update Surface")) {
            planSize = planSizeTemp;
            renderSurface();
        }
        gridSettings.var("Roughness", roughness, 0.f,1.f);
    }


    {
        auto loadGroup = w.group("Load Options");
        loadGroup.checkbox("Use Original Tangents", mUseOriginalTangents);
        loadGroup.tooltip("If this is unchecked, we will ignore the tangents that were loaded from the model and calculate them internally. Check this box if you'd like to use the original tangents");
        loadGroup.checkbox("Don't Merge Materials", mDontMergeMaterials);
        loadGroup.tooltip("Don't merge materials that have the same properties. Use this option to preserve the original material names.");
    }

    {
        auto orthQuadSettings = w.group("Orthographic Camera Settings");
        orthQuadSettings.var("OrthoQuad Rotate", rotateQuad, 0.f, 90.f);
        orthQuadSettings.var("OrthoQuad Position", orthoCamPostion, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 0.1f);
        orthQuadSettings.var("OrthoQuad Size", cameraSize, 0.f, 2.f);
    }
    w.var("Samples", sampleNum, 3);
    w.var("Bounces", bounces,0, 100);

    w.separator();




    Gui::DropdownList cameraDropdown;
    cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::FirstPerson, "First-Person" });
    cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::Orbiter, "Orbiter" });
    cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::SixDOF, "6-DoF" });

    if (w.dropdown("Camera Type", cameraDropdown, (uint32_t&)mCameraType)) setCamController();

    if (w.checkbox("Orthographic View", mOrthoCam)) {
        
        resetCamera();
    }
    if(mOrthoCam){
        auto orthCameraSettings = w.group("Orthographic Camera Settings");
        int tempWidth = orthCamWidth;
        int tempHeight = orthCamHeight;
        orthCameraSettings.var("Width", orthCamWidth, 10);
        orthCameraSettings.var("Height", orthCamHeight, 10);
        if (tempWidth != orthCamWidth || tempHeight != orthCamHeight) {
            tempWidth = orthCamWidth;
            tempHeight = orthCamHeight;
        }
       // resetCamera();
    }

    w.separator();
    startSimulation = w.button("Start Simulation");
    normalSim = w.button("Normal Simulation");
    clearTexture = w.button("Clear Texture");
}

void BRDF_Simulator::onLoad(RenderContext* pRenderContext)
{
    
    mpGraphicsState = GraphicsState::create();
    mpOrthoCubeGraphicsState = GraphicsState::create();
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


    /*ORTHOCAMERA SETTINGS*/


    /*___________________________________________________________________*/
        // Create the rasterizer state
    DepthStencilState::Desc tempdsDesc;
    tempdsDesc.setDepthFunc(ComparisonFunc::Less).setDepthEnabled(true);
    mpOrthoCubeDepthTestDS = DepthStencilState::create(tempdsDesc);


    /*----------------------------*/
   
    resetCamera();
    loadOrthoQuad();
    setEnvMapPipeline();
    renderSurface();
    
    mpScene->getMaterialSystem()->setDefaultTextureSampler(mpPointSampler);
    mpOrthoCubeScene->getMaterialSystem()->setDefaultTextureSampler(mpLinearSampler);
    //FIX
    //orthoLeft = -40.0f * mpScene->getCamera()->getAspectRatio();
    //orthoRight = 40.0f * mpScene->getCamera()->getAspectRatio();
    //orthoTop = -40.0f;
    //orthoBottom = 40.0f;
    //pixelsNum = (orthoRight - orthoLeft) * (orthoBottom - orthoTop);
}



void BRDF_Simulator::onFrameRender(RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo)
{

    const float4 clearColor(0.4f, 0.4f, 0.4f, 1);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);
    mpOrthoCubeGraphicsState->setFbo(pTargetFbo);

    mpGraphicsState->setFbo(pTargetFbo);

    if (clearTexture) {
        clearTexture = !clearTexture;
        setEnvMapPipeline();
    }

    mpCubeGraphicsState->setFbo(pTargetFbo);
    setOrthoCubeVars();
    setEnvMapShaderVars();
    
    if (mpScene)
    {
        mpScene->update(pRenderContext, gpFramework->getGlobalClock().getTime());

        // Set render state

        
        {
            mpOrthoCubeGraphicsState->setDepthStencilState(mpOrthoCubeDepthTestDS);

            mpGraphicsState->setDepthStencilState(mpDepthTestDS);
            mpProgramVars["PerFrameCB"]["gConstColor"] = false;
            mpProgramVars["PerFrameCB"]["simulate"] = false;// this->startSimulation;
            mpProgramVars["PerFrameCB"]["normalSim"] = normalSim;
            mpProgramVars["PerFrameCB"]["roughness"] = roughness;
            mpProgramVars["PerFrameCB"]["surfaceSize"] = planSize;
            mpProgramVars["PerFrameCB"]["bounces"] = bounces;
            mpProgramVars["PerFrameCB"]["orthCamWidth"] = orthCamWidth;
            mpProgramVars["PerFrameCB"]["orthCamHeight"] = orthCamHeight;
            mpProgramVars["PerFrameCB"]["nearPlanePos"] = int(mpScene->getCamera()->getNearPlane());

            mpOrthoCubeProgramVars["PerFrameCB"]["simulate"] = this->startSimulation;
            mpOrthoCubeProgramVars["PerFrameCB"]["surfaceSize"] = planSize;
            mpOrthoCubeProgramVars["PerFrameCB"]["roughness"] = roughness;
            const auto& pEnvMap = mpCubeScene->getEnvMap();
            if (pEnvMap) {
                mpProgramVars["PerFrameCB"]["tex2D_uav"].setUav(pEnvMap->getEnvMap()->getUAV(0));
                //setUav(pEnvMap->getEnvMap()->getUAV(0));
                mpProgramVars["PerFrameCB"]["envSampler"].setSampler(pEnvMap->getEnvSampler());

                mpProgramVars["PerFrameCB"]["samples"] = sampleNum;

                mpOrthoCubeProgramVars["PerFrameCB"]["tex2D_uav"].setUav(pEnvMap->getEnvMap()->getUAV(0));
                mpOrthoCubeProgramVars["PerFrameCB"]["envSampler"].setSampler(pEnvMap->getEnvSampler());
                //std::cout <<  sampleNum << std::endl;
            }

            mpOrthoCubeScene->rasterize(pRenderContext, mpOrthoCubeGraphicsState.get(), mpOrthoCubeProgramVars.get(), mpOrthoCubeWireframeRS, mpOrthoCubeWireframeRS);
            mpScene->rasterize(pRenderContext, mpGraphicsState.get(), mpProgramVars.get(), RasterizerState::CullMode::None);
            mpCubeScene->rasterize(pRenderContext, mpCubeGraphicsState.get(), mpCubeProgramVars.get(), mpRsState, mpRsState);
            
        }
    }


    if (mOrthoCam) {
        mpScene->getCamera()->setProjectionMatrix(Falcor::rmcv::ortho(float(-orthCamWidth), float(orthCamWidth), float(-orthCamHeight), float(orthCamHeight), mpScene->getCamera()->getNearPlane(), mpScene->getCamera()->getFarPlane()));
        std::cout << "Postion of orthoCam: (" + std::to_string(mpScene->getCamera()->getPosition()[0]) + "," + std::to_string(mpScene->getCamera()->getPosition()[1]) + ", " + std::to_string(mpScene->getCamera()->getPosition()[2]) + ")" << std::endl;
        std::cout << "Left, right, top, buttom : (" + std::to_string(mpScene->getCamera()->getFarPlane())  + ")" << std::endl;

    }
    else {
        mpScene->getCamera()->setProjectionMatrix(Falcor::rmcv::perspective(Falcor::focalLengthToFovY(mpScene->getCamera()->getFocalLength(), mpScene->getCamera()->getFrameHeight()), mpScene->getCamera()->getFrameWidth()/ mpScene->getCamera()->getFrameHeight(), mpScene->getCamera()->getNearPlane(), mpScene->getCamera()->getFarPlane()));

    }
    TextRenderer::render(pRenderContext, mModelString, pTargetFbo, float2(10, 30));

    // change the simulation boolean to false.
    this->startSimulation = false;
    normalSim = false;
}

bool BRDF_Simulator::onKeyEvent(const KeyboardEvent& keyEvent)
{
    if (mOrthoCam && (keyEvent.key == Input::Key::W || keyEvent.key == Input::Key::S)) return false;
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
    if (mOrthoCam && mouseEvent.type == MouseEvent::Type::Wheel) return false;
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
    if (mpOrthoCubeScene) { mpOrthoCubeScene->setCameraController(mCameraType); }

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
