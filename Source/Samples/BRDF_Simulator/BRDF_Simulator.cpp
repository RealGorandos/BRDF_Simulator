#include "BRDF_Simulator.h"
#include "Utils/UI/TextRenderer.h"
#include "Utils/Math/FalcorMath.h"
#include <random>


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

    auto pTex = Texture::create2D(50, 50, ResourceFormat::R32Uint, 1, 1, nullptr, ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess);
    pTex->setName("Enviroment Map");
    
    mpCubeScene->setEnvMap(EnvMap::create(pTex));
    mpCubeScene->getMaterialSystem()->setDefaultTextureSampler(mpCubePointSampler);

}


void BRDF_Simulator::loadOrthoQuad() {

    DepthStencilState::Desc tempdsDesc;
    tempdsDesc.setDepthFunc(ComparisonFunc::Less).setDepthEnabled(true);
    mpDebuggingQuadDepthTestDS = DepthStencilState::create(tempdsDesc);

    mpDebuggingQuadSceneBuilder = SceneBuilder::create(SceneBuilder::Flags::None);
    SceneBuilder::Node N;
    N.transform[0][3] = float(currLayer);
    N.transform[1][3] = float(currLayer);
    N.transform[2][3] = float(planSize[0] + 2) / 2.f;
    Falcor::StandardMaterial::SharedPtr Material = StandardMaterial::create("Surface Material", ShadingModel::MetalRough);
    float width = 60;
    float height = 30;
    Falcor::TriangleMesh::SharedPtr quadTemp = TriangleMesh::createQuad(float2(width, height));
    orthCamWidth = width / 2.f;
    orthCamHeight = height / 2.f;
    quadTemp->addVertex( float3(-(0.5f * width), 50, -(0.5f * height)), float3(0.f, 0.f, 0.f), float2( 0.f, 0.f ));
    quadTemp->addVertex({ (0.5f * width), 50, -(0.5f * height) }, float3(0.f, 0.f, 0.f), { 1.f, 0.f });
    quadTemp->addVertex({ -(0.5f * width), 50,  (0.5f * height) }, float3(0.f, 0.f, 0.f), { 0.f, 1.f });
    quadTemp->addVertex({ (0.5f * width), 50,  (0.5f * height) }, float3(0.f, 0.f, 0.f), { 1.f, 1.f });

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

    // quadTemp->setFrontFaceCW(true);
    Falcor::Transform quadTranform = Falcor::Transform::Transform();
    quadTranform.setRotationEuler(float3(0.f, 0.f, -1.570800));
    N.transform = N.transform * rmcv::mat4_cast(quadTranform.getRotation());
    quadTranform.lookAt(float3(N.transform[0][3], N.transform[1][3], N.transform[2][3]), float3(float(planSize[0] + 2) / 2.f, 0.f, float(planSize[0] + 2) / 2.f), up);

    std::string Mat = "";

    N.transform = N.transform * rmcv::mat4_cast(quadTranform.getRotation());
    quadTranform.setRotationEulerDeg(rotateQuad);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            Mat += " " + std::to_string(N.transform[i][j]);
        }
        Mat += "\n";
    }
    std::cout << Mat << std::endl;


    mpDebuggingQuadSceneBuilder->addMeshInstance(mpDebuggingQuadSceneBuilder->addNode(N), mpDebuggingQuadSceneBuilder->addTriangleMesh(quadTemp, Material));


    mpDebuggingQuadScene = mpDebuggingQuadSceneBuilder->getScene();

    {
        Program::Desc desc;
        desc.addShaderModules(mpDebuggingQuadScene->getShaderModules());
        desc.addShaderLibrary("Samples/BRDF_Simulator/OrthoCam.3d.hlsl").vsEntry("vsMain").psEntry("psMain");
        desc.addTypeConformances(mpDebuggingQuadScene->getTypeConformances());

        mpDebuggingQuadProgram = GraphicsProgram::create(desc, mpDebuggingQuadScene->getSceneDefines());
    }

    mpDebuggingQuadProgramVars = GraphicsVars::create(mpDebuggingQuadProgram->getReflector());




    // Create rasterizer state
    RasterizerState::Desc orthoCubewireframeDesc;
    orthoCubewireframeDesc.setFillMode(RasterizerState::FillMode::Wireframe);
    orthoCubewireframeDesc.setCullMode(RasterizerState::CullMode::None);//.setDepthClamp(true);
   // orthoCubewireframeDesc.setFrontCounterCW(false);
    mpDebuggingQuadWireframeRS = RasterizerState::create(orthoCubewireframeDesc);

    // Depth test
    mpDebuggingQuadGraphicsState->setProgram(mpDebuggingQuadProgram);
    setCamController();
    mpDebuggingQuadScene->getMaterialSystem()->setDefaultTextureSampler(mpLinearSampler);
}

void BRDF_Simulator::setEnvMapShaderVars() {

    const auto& pEnvMap = mpCubeScene->getEnvMap();
    mpCubeProgram->addDefine("_USE_ENV_MAP", pEnvMap ? "1" : "0");
    if (pEnvMap) {
        mpCubeProgramVars["PerFrameCB"]["tex2D_uav"].setTexture(pEnvMap->getEnvMap());
        mpCubeProgramVars["PerFrameCB"]["gSamples"] = jitterInternal;
            //setUav(pEnvMap->getEnvMap()->getUAV(0));
        mpCubeProgramVars["PerFrameCB"]["envSampler"].setSampler(pEnvMap->getEnvSampler());
        //pEnvMap->setShaderData(mpCubeProgramVars["PerFrameCB"]["gEnvMap"]);
    }

    rmcv::mat4 world = rmcv::translate(mpScene->getCamera()->getPosition());

    mpCubeProgramVars["PerFrameCB"]["gWorld"] = world;
    mpCubeProgramVars["PerFrameCB"]["gScale"] = 1.f;
    mpCubeProgramVars["PerFrameCB"]["gViewMat"] = mpScene->getCamera()->getViewMatrix();
    mpCubeProgramVars["PerFrameCB"]["gProjMat"] = mpCubeScene->getCamera()->getProjMatrix();
    mpCubeProgramVars["PerFrameCB"]["gObjSimulation"] = mObjectSimulation;
}

void BRDF_Simulator::setOrthoCubeVars() {
   // mpDebuggingQuadScene->setCamera(mpScene->getCamera());
    rmcv::mat4 world = rmcv::translate(float3(0.f));
    //mpDebuggingQuadScene->setCamera(mpScene->getCamera());
    mpDebuggingQuadProgramVars["PerFrameCB"]["gWorld"] = world;
   // mpDebuggingQuadProgramVars["PerFrameCB"]["surfaceWorldMat"] = rmcv::translate(mpScene->getCamera()->getPosition());
    //mpDebuggingQuadProgramVars["PerFrameCB"]["gScale"] = 1.f;
    mpDebuggingQuadProgramVars["PerFrameCB"]["gViewMat"] = mpScene->getCamera()->getViewMatrix();
    mpDebuggingQuadProgramVars["PerFrameCB"]["gProjMat"] = mpScene->getCamera()->getProjMatrix();
    mpDebuggingQuadProgramVars["PerFrameCB"]["gConstColor"] = false;

    mpDebuggingQuadProgramVars["PerFrameCB"]["rotation"] = rotateQuad;
    mpDebuggingQuadProgramVars["PerFrameCB"]["cameraSize"] = cameraSize;
    mpDebuggingQuadProgramVars["PerFrameCB"]["camPosition"] = orthoCamPostion;
    mpDebuggingQuadProgramVars["PerFrameCB"]["simulate"] = this->debuggingQuad;
    mpDebuggingQuadProgramVars["PerFrameCB"]["surfaceSize"] = planSize;
    mpDebuggingQuadProgramVars["PerFrameCB"]["roughness"] = roughness;
    const auto& pEnvMap = mpCubeScene->getEnvMap();
    if (pEnvMap) {
        mpDebuggingQuadProgramVars["PerFrameCB"]["tex2D_uav"].setUav(pEnvMap->getEnvMap()->getUAV(0));
        mpDebuggingQuadProgramVars["PerFrameCB"]["envSampler"].setSampler(pEnvMap->getEnvSampler());
    }
}

void BRDF_Simulator::setSceneVars() {
    mpProgramVars["PerFrameCB"]["gConstColor"] = false;
    mpProgramVars["PerFrameCB"]["simulate"] = false;// this->startSimulation;
    mpProgramVars["PerFrameCB"]["BRDF_Simulation"] = BRDF_Simulation && switchBool;
    mpProgramVars["PerFrameCB"]["roughness"] = roughness;
    mpProgramVars["PerFrameCB"]["surfaceSize"] = planSize;
    mpProgramVars["PerFrameCB"]["bounces"] = bounces;
    mpProgramVars["PerFrameCB"]["orthCamWidth"] = orthCamWidth;
    mpProgramVars["PerFrameCB"]["orthCamHeight"] = orthCamHeight;
    mpProgramVars["PerFrameCB"]["nearPlanePos"] = int(mpScene->getCamera()->getNearPlane());
    mpProgramVars["PerFrameCB"]["bounces"] = bounces;
    mpProgramVars["PerFrameCB"]["LoadedObj"] = mObjectSimulation;
    mpProgramVars["PerFrameCB"]["c_pos"] = mpScene->getCamera()->getTarget() - mpScene->getCamera()->getPosition();
    //This gWorld Might not work!!
    mpProgramVars["PerFrameCB"]["gWorld"] = rmcv::translate(mpScene->getCamera()->getPosition());
    const auto& pEnvMap = mpCubeScene->getEnvMap();
    if (pEnvMap) {
        mpProgramVars["PerFrameCB"]["tex2D_uav"].setUav(pEnvMap->getEnvMap()->getUAV(0));
        //setUav(pEnvMap->getEnvMap()->getUAV(0));
        mpProgramVars["PerFrameCB"]["envSampler"].setSampler(pEnvMap->getEnvSampler());

        mpProgramVars["PerFrameCB"]["samples"] = jitterInternal;

    }
}

void BRDF_Simulator::renderSurface() {

   // CpuTimer timer;
   // timer.update();

    mSceneBuilder = SceneBuilder::create(SceneBuilder::Flags::None);
    SceneBuilder::Node N;
    Falcor::StandardMaterial::SharedPtr Material = StandardMaterial::create("Surface Material", ShadingModel::MetalRough);
    for (int row = 0; row < planSize[0]; row++) {
        for (int col = 0; col < planSize[1]; col++) {
            N.transform[0][3] = float(col) + 1.5;
            N.transform[2][3] = float(row) + 1.5;
            mSceneBuilder->addMeshInstance(mSceneBuilder->addNode(N), mSceneBuilder->addTriangleMesh(TriangleMesh::createQuad(float2(1.f)), Material));
        }
    }
    std::cout << "------------------------col: " + std::to_string(planSize[1]) << std::endl;
    std::cout << "------------------------row: " + std::to_string(planSize[0]) << std::endl;

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
    //timer.update();
   // setModelString(timer.delta());
    mpScene->getMaterialSystem()->setDefaultTextureSampler(mpPointSampler);
}




void BRDF_Simulator::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "BRDF Simulator Demo", { 400, 300 }, { 0, 100 });


    w.separator();



    if (mMicrofacetes) {

        {
            auto gridSettings = w.group("Surface Settings");
           // gridSettings.var("Surface Size", planSizeTemp, 1);
            //if (gridSettings.button("Update Surface")) {
                //planSize = planSizeTemp;
                //renderSurface();
               // loadOrthoQuad();
           // }
            gridSettings.var("Roughness", roughness, 0.f, 1.f);
        }
        //w.separator();
        //w.checkbox("Show Debugging Quad", showDebuggingQuad);
        //if(showDebuggingQuad){
        //    auto orthQuadSettings = w.group("Debugging Quad Settings");
        //    orthQuadSettings.var("Quad Pos", orthoCamPostionTemp);
        //    orthQuadSettings.var("Quad LookAt", QuadLookAtTemp);
        //    orthQuadSettings.var("Quad up", upTemp);
        //    orthQuadSettings.var("rotate quad Euler", rotateQuadTemp);
        //    if (w.button("Apply Look At")) {
        //        QuadLookAt = QuadLookAtTemp;
        //        orthoCamPostion = orthoCamPostionTemp;
        //        up = upTemp;
        //        rotateQuad = rotateQuadTemp;
        //        loadOrthoQuad();
        //    }
        //    
        //}

        w.separator();
        
    
        w.var("Jitter", jitterNum, 2);
        w.var("Bounces", bounces, 1);
        


        w.separator();

        Gui::DropdownList cameraDropdown;
        cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::FirstPerson, "First-Person" });
        cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::Orbiter, "Orbiter" });
        cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::SixDOF, "6-DoF" });

        if (w.dropdown("Camera Type", cameraDropdown, (uint32_t&)mCameraType)) setCamController();




        
        w.separator();
        w.var("Current Layer", currLayer, 1, 30);


        w.separator();
        //bool temp = ;
        if (w.button("Draw To Texture")) {
            BRDF_Simulation = true;
            mOrthoCam = true;
            jitterInternal = jitterNum;
            bouncesInternal = bounces;
            camCurrPos = mpScene->getCamera()->getPosition();
            mpScene->getCamera()->setPosition(float3(float(currLayer), float(currLayer), float(planSize[0] + 2) / 2.f));
            mpScene->getCamera()->setTarget(float3(float(planSize[0] + 2) / 2.f, 0.f, float(planSize[0] + 2) / 2.f));
            mpScene->getCamera()->setUpVector(float3(0, 1, 0));
            
        }
        clearTexture = w.button("Clear Texture");

        w.separator();
    }

        


    if (mObjectSimulation) {



        Gui::DropdownList cameraDropdown;
        cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::FirstPerson, "First-Person" });
        cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::Orbiter, "Orbiter" });
        cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::SixDOF, "6-DoF" });

        if (w.dropdown("Camera Type", cameraDropdown, (uint32_t&)mCameraType)) setCamController();

        w.separator();
        Gui::DropdownList brdfDropdown;
        brdfDropdown.push_back({ (uint32_t)BRDF_Simulator::BRDF_Type::BRDF_Simulation, "BRDF Simulation" });
        brdfDropdown.push_back({ (uint32_t)BRDF_Simulator::BRDF_Type::Cook_Torrance, "Cook Torrance" });
        if (w.dropdown("BRDF Type", brdfDropdown, (uint32_t&)mBRDFType)) {
            switch (mBRDFType) {
            case BRDF_Simulator::BRDF_Type::Cook_Torrance:
                isCookTorrence = true;
                isSimulation = false;
                break;
            case BRDF_Simulator::BRDF_Type::BRDF_Simulation:
                isSimulation = true;
                isCookTorrence = false;
                break;
            default:
                FALCOR_UNREACHABLE();
            }
        }
        
        

        w.separator();
        if (isCookTorrence) {
            w.var("Metallic", metallic, 0.f, 1.f);
            w.var("Roughness", mRoughness, 0.f, 1.f);
            w.var("Albedo", mAlbedo, 0.f, 1.f);
            
           // w.var("ao", ao, 0.f, 1.f);
           // w.var("Albedo", jitterNum, 2);
        }
        w.separator();

        if (w.button("Start Simulation")) {
             runSimulation = true;
        }

    }

    
    //startSimulation = w.button("Start Simulation");


    
    if (!mObjectSimulation) {
        if (w.button("View Model"))
        {
            mObjectSimulation = true;
            mMicrofacetes = false;
            loadModel(gpFramework->getTargetFbo()->getColorTexture(0)->getFormat());
        }
    }

    if (!mMicrofacetes) {
        if (w.button("View Surface"))
        {
            mMicrofacetes = true;
            mObjectSimulation = false;
            renderSurface();
          //  loadOrthoQuad();
        }
    }

}
void BRDF_Simulator::loadModelFromFile(const std::filesystem::path& path, ResourceFormat fboFormat)
{
    //CpuTimer timer;
    //timer.update();

    SceneBuilder::Flags flags = SceneBuilder::Flags::None;
    if (mUseOriginalTangents) flags |= SceneBuilder::Flags::UseOriginalTangentSpace;
    if (mDontMergeMaterials) flags |= SceneBuilder::Flags::DontMergeMaterials;
    flags |= isSrgbFormat(fboFormat) ? SceneBuilder::Flags::None : SceneBuilder::Flags::AssumeLinearSpaceTextures;

    try
    {
        mpModelScene = SceneBuilder::create(path, flags)->getScene();
    }
    catch (const std::exception& e)
    {
        msgBox(fmt::format("Failed to load model.\n\n{}", e.what()));
        return;
    }

    Program::Desc desc;
    desc.addShaderModules(mpModelScene->getShaderModules());
    desc.addShaderLibrary("Samples/BRDF_Simulator/Model.3d.hlsl").vsEntry("vsMain").psEntry("psMain");
    desc.addTypeConformances(mpModelScene->getTypeConformances());

    mpModelProgram = GraphicsProgram::create(desc, mpModelScene->getSceneDefines());
    mpModelProgramVars = GraphicsVars::create(mpModelProgram->getReflector());
    mpModelGraphicsState->setProgram(mpModelProgram);

    mpModelScene->getMaterialSystem()->setDefaultTextureSampler(mUseTriLinearFiltering ? mpLinearSampler : mpPointSampler);
    setCamController();

    //timer.update();
    //setModelString(timer.delta());
}
void BRDF_Simulator::loadModel(ResourceFormat fboFormat)
{
    std::filesystem::path path;
    if (openFileDialog(Scene::getFileExtensionFilters(), path))
    {
        loadModelFromFile(path, fboFormat);
    }
}

void BRDF_Simulator::onLoad(RenderContext* pRenderContext)
{
    
    mpGraphicsState = GraphicsState::create();
    mpDebuggingQuadGraphicsState = GraphicsState::create();
    mpModelGraphicsState = GraphicsState::create();
    // Depth test
    DepthStencilState::Desc dsDesc;
    dsDesc.setDepthEnabled(false);
    mpNoDepthDS = DepthStencilState::create(dsDesc);
    dsDesc.setDepthFunc(ComparisonFunc::Less).setDepthEnabled(true);
    mpDepthTestDS = DepthStencilState::create(dsDesc);

    Sampler::Desc samplerDesc;
    samplerDesc.setFilterMode(Sampler::Filter::Point, Sampler::Filter::Point, Sampler::Filter::Point);
    mpPointSampler = Sampler::create(samplerDesc);
    samplerDesc.setFilterMode(Sampler::Filter::Linear, Sampler::Filter::Linear, Sampler::Filter::Linear);
    mpLinearSampler = Sampler::create(samplerDesc);

   
    resetCamera();
    renderSurface();
    loadOrthoQuad();
    setEnvMapPipeline();

   // if (mMicrofacetes) {
        
   // }
    //loadModel(ResourceFormat fboFormat)
    //
    
    
    

}
void BRDF_Simulator::setModelVars() {
    mpModelProgramVars["PerFrameCB"]["camPos"] = mpModelScene->getCamera()->getPosition();
    //Material
    mpModelProgramVars["PerFrameCB"]["roughness"] = mRoughness;
    mpModelProgramVars["PerFrameCB"]["metallic"] = metallic;
    mpModelProgramVars["PerFrameCB"]["albedo"] = mAlbedo;
    mpModelProgramVars["PerFrameCB"]["ao"] = ao;
    //Simulation Type
    mpModelProgramVars["PerFrameCB"]["startBrdf"] = runSimulation;
    mpModelProgramVars["PerFrameCB"]["cookTorrence"] = isCookTorrence;
    
}
float get_random()
{
    //std::cout << (1.f/float(Falcor::getDisplayDpi()))/2.f << std::endl;
    static std::default_random_engine e;
    static std::uniform_real_distribution<> dis(-(1.f / float(Falcor::getDisplayDpi())) / 2.f, (1.f / float(Falcor::getDisplayDpi())) / 2.f); // rage 0 - 1
    return dis(e);
}

void BRDF_Simulator::onFrameRender(RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo)
{

    const float4 clearColor(0.4f, 0.4f, 0.4f, 1);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);
    mpDebuggingQuadGraphicsState->setFbo(pTargetFbo);
    mpModelGraphicsState->setFbo(pTargetFbo);
    mpGraphicsState->setFbo(pTargetFbo);
    mpCubeGraphicsState->setFbo(pTargetFbo);

    if (mpModelScene && !mMicrofacetes) {
        mpModelScene->update(pRenderContext, gpFramework->getGlobalClock().getTime());
        mpModelGraphicsState->setDepthStencilState(mpDepthTestDS);
        setModelVars();
        mpModelScene->rasterize(pRenderContext, mpModelGraphicsState.get(), mpModelProgramVars.get(), RasterizerState::CullMode::None);

    }
    else if (mpScene && !mObjectSimulation)
    {
        // change the simulation boolean to false.

        if (jitterInternal <= 1) {
            mpScene->getCamera()->setJitter(0, 0);
            this->switchBool = false;
            this->debuggingQuad = false;
            this->BRDF_Simulation = false;
            this->mOrthoCam = false;
        }

        if (jitterInternal > 1 && BRDF_Simulation && !switchBool) {

            mpScene->getCamera()->setPosition(float3(float(currLayer), float(currLayer), float(planSize[0] + 2) / 2.f));
            mpScene->getCamera()->setTarget(float3(float(planSize[0] + 2) / 2.f, 0.f, float(planSize[0] + 2) / 2.f));
            mpScene->getCamera()->setUpVector(float3(0, 1, 0));

            float rand01 = get_random();
            float rand02 = get_random();

            mpScene->getCamera()->setJitter(rand01, rand02);
            switchBool = true;
            this->mOrthoCam = true;



            jitterInternal--;
        } else if (BRDF_Simulation && switchBool) {
        std::cout << "We are in the switch Condition!!!!!!!!" << std::endl;
        //resetCamera();
        this->mOrthoCam = false;
        switchBool = false;
        mpScene->getCamera()->setPosition(float3(0.f,0.f,0.f));
        mpScene->getCamera()->setTarget(float3(float(planSize[0] + 2) / 2.f, 0.f, float(planSize[0] + 2) / 2.f));
        mpScene->getCamera()->setUpVector(float3(0, 1, 0));

        }


        if (currLayerTemp != currLayer) {
            currLayerTemp = currLayer;
            loadOrthoQuad();
        }
     //   loadOrthoQuad();
        if (clearTexture) {
            clearTexture = !clearTexture;
            setEnvMapPipeline();
        }
        mpScene->update(pRenderContext, gpFramework->getGlobalClock().getTime());
        mpDebuggingQuadScene->update(pRenderContext, gpFramework->getGlobalClock().getTime());
        // Set render state     
        {
            mpDebuggingQuadGraphicsState->setDepthStencilState(mpDebuggingQuadDepthTestDS);
            mpGraphicsState->setDepthStencilState(mpDepthTestDS);
            setSceneVars();
            
            setOrthoCubeVars();
            setEnvMapShaderVars();

            if (mOrthoCam) {
                mpScene->getCamera()->setProjectionMatrix(Falcor::rmcv::ortho(float(-orthCamWidth), float(orthCamWidth), float(-orthCamHeight), float(orthCamHeight), mpScene->getCamera()->getNearPlane(), mpScene->getCamera()->getFarPlane()));

            }
            else {
                mpScene->getCamera()->setProjectionMatrix(Falcor::rmcv::perspective(Falcor::focalLengthToFovY(mpScene->getCamera()->getFocalLength(), mpScene->getCamera()->getFrameHeight()), mpScene->getCamera()->getFrameWidth() / mpScene->getCamera()->getFrameHeight(), mpScene->getCamera()->getNearPlane(), mpScene->getCamera()->getFarPlane()));

            }
            if (showDebuggingQuad) {
                mpDebuggingQuadScene->rasterize(pRenderContext, mpDebuggingQuadGraphicsState.get(), mpDebuggingQuadProgramVars.get(), mpDebuggingQuadWireframeRS, mpDebuggingQuadWireframeRS);
            }
            
            if (mOrthoCam) {
                    mpScene->rasterize(pRenderContext, mpGraphicsState.get(), mpProgramVars.get(), RasterizerState::CullMode::Back);


            }
            else {
                mpScene->rasterize(pRenderContext, mpGraphicsState.get(), mpProgramVars.get(), RasterizerState::CullMode::None);
            }
            
            mpCubeScene->rasterize(pRenderContext, mpCubeGraphicsState.get(), mpCubeProgramVars.get(), mpRsState, mpRsState);
            
        }


    }



    TextRenderer::render(pRenderContext, mModelString, pTargetFbo, float2(10, 30));
    //if (BRDF_Simulation) {
    //    switchBool = !switchBool;
    //}
    




    
}

bool BRDF_Simulator::onKeyEvent(const KeyboardEvent& keyEvent)
{

    if (mpModelScene && !mMicrofacetes) {
        if (mpModelScene && mpModelScene->onKeyEvent(keyEvent)) return true;
        if ((keyEvent.type == KeyboardEvent::Type::KeyPressed) && (keyEvent.key == Input::Key::R))
        {
            resetCamera();
            return true;
        }
    }
    else if (mpScene && !mObjectSimulation) {
        if (mOrthoCam && (keyEvent.key == Input::Key::W || keyEvent.key == Input::Key::S)) return false;
        if (mpScene && mpScene->onKeyEvent(keyEvent)) return true;

        if ((keyEvent.type == KeyboardEvent::Type::KeyPressed) && (keyEvent.key == Input::Key::R))
        {
            resetCamera();
            return true;
        }


    }

    return false;
}

bool BRDF_Simulator::onMouseEvent(const MouseEvent& mouseEvent)
{
    //if (mOrthoCam && mouseEvent.type == MouseEvent::Type::Wheel) return false;
    if (mpModelScene && !mMicrofacetes) {
        mpModelScene->onMouseEvent(mouseEvent);
    }
    else if (mpScene && !mObjectSimulation) {
        if (mOrthoCam && mouseEvent.type == MouseEvent::Type::Wheel) return false;
        return mpScene->onMouseEvent(mouseEvent);
    }
    return false;
}

void BRDF_Simulator::onResizeSwapChain(uint32_t width, uint32_t height)
{
    float h = (float)height;
    float w = (float)width;
}

void BRDF_Simulator::setCamController()
{
    if(mpModelScene  && !mMicrofacetes) { mpModelScene->setCameraController(mCameraType); }
    else if (mpScene && !mObjectSimulation) {mpScene->setCameraController(mCameraType);}
    //if (mpDebuggingQuadScene) { mpDebuggingQuadScene->setCameraController(mCameraType); }

}



void BRDF_Simulator::resetCamera()
{
    if (mpModelScene &&!mMicrofacetes) {
        mpModelScene->resetCamera(true);
        setCamController();
    }
    else if (mpScene && !mObjectSimulation)
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
