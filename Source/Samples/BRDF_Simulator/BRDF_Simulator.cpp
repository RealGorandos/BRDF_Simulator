#include "BRDF_Simulator.h"
#include "Utils/UI/TextRenderer.h"
#include "Utils/Math/FalcorMath.h"
#include <math.h>
#include <random>
#include <Utils/StringUtils.h>


#pragma region HELPER METHODS SECTION
///////////////////_____________HELPER METHODS SECTION__________________/////////////////
/*Printing data in the window*/
void BRDF_Simulator::setModelString(double loadTime)
{
    FALCOR_ASSERT(mpScene != nullptr);

    mModelString = "Loading took " + std::to_string(loadTime) + " seconds.\n";
    mModelString += std::to_string(mpScene->getMeshCount()) + " meshes, ";
    mModelString += std::to_string(mpScene->getGeometryInstanceCount()) + " instances, ";
    mModelString += std::to_string(mpScene->getMaterialCount()) + " materials, ";
}
/*Gets a random number*/
float get_random()
{

    static std::default_random_engine e;
    static std::uniform_real_distribution<> dis(-0.5f, 0.5f); // rage 0 - 1
    return dis(e);
}
/////______________________________________________________________________________________________________________
#pragma endregion HELPER METHODS

#pragma region SHADER VARS FILLING
/////////////___FILLING THE SHADERS VARIABLES SECTION__///////////////////////////////////
void BRDF_Simulator::setEnvMapShaderVars() {

    const auto& pEnvMap = mpCubeScene->getEnvMap();
    mpCubeProgram->addDefine("_USE_ENV_MAP", pEnvMap ? "1" : "0");
    if (pEnvMap) {
        mpCubeProgramVars["PerFrameCB"]["tex2D_uav"].setTexture(textureVect[currLayer - 1]->getEnvMap());
        mpCubeProgramVars["PerFrameCB"]["gSamples"] = jitterInternal;
        mpCubeProgramVars["PerFrameCB"]["envSampler"].setSampler(textureVect[currLayer - 1]->getEnvSampler());
    }

    rmcv::mat4 world = rmcv::translate(mpScene->getCamera()->getPosition());

    mpCubeProgramVars["PerFrameCB"]["gWorld"] = world;
    mpCubeProgramVars["PerFrameCB"]["gScale"] = 1.f;
    mpCubeProgramVars["PerFrameCB"]["gViewMat"] = mpScene->getCamera()->getViewMatrix();
    mpCubeProgramVars["PerFrameCB"]["gProjMat"] = mpCubeScene->getCamera()->getProjMatrix();
}

void BRDF_Simulator::setEnvMapModelShaderVars() {

    const auto& pEnvMap = mpCubeScene->getEnvMap();
    mpCubeProgram->addDefine("_USE_ENV_MAP", pEnvMap ? "1" : "0");
    if (pEnvMap) {
        mpCubeProgramVars["PerFrameCB"]["tex2D_uav"].setTexture(textureVect[currLayer - 1]->getEnvMap());
        mpCubeProgramVars["PerFrameCB"]["gSamples"] = jitterInternal;
        mpCubeProgramVars["PerFrameCB"]["envSampler"].setSampler(textureVect[currLayer - 1]->getEnvSampler());
    }

    rmcv::mat4 world = rmcv::translate(mpModelScene->getCamera()->getPosition());

    mpCubeProgramVars["PerFrameCB"]["gWorld"] = world;
    mpCubeProgramVars["PerFrameCB"]["gScale"] = 1.f;
    mpCubeProgramVars["PerFrameCB"]["gViewMat"] = mpModelScene->getCamera()->getViewMatrix();
    mpCubeProgramVars["PerFrameCB"]["gProjMat"] = mpCubeScene->getCamera()->getProjMatrix();
}

void BRDF_Simulator::setOrthoVisualizorVars() {

    rmcv::mat4 world = rmcv::translate(float3(0.f));
    mpDebuggingQuadProgramVars["PerFrameCB"]["gWorld"] = world;
    mpDebuggingQuadProgramVars["PerFrameCB"]["gViewMat"] = mpScene->getCamera()->getViewMatrix();
    mpDebuggingQuadProgramVars["PerFrameCB"]["gProjMat"] = mpScene->getCamera()->getProjMatrix();
}

void BRDF_Simulator::setSceneVars() {
    mpProgramVars["PerFrameCB"]["BRDF_Simulation"] = BRDF_Simulation && switchBool;
    mpProgramVars["PerFrameCB"]["roughness"] = roughness;
    mpProgramVars["PerFrameCB"]["surfaceSize"] = planSize;
    mpProgramVars["PerFrameCB"]["bounces"] = bounces;
    mpProgramVars["PerFrameCB"]["c_dir"] = mpScene->getCamera()->getTarget() - mpScene->getCamera()->getPosition();
    const auto& pEnvMap = mpCubeScene->getEnvMap();
    if (pEnvMap) {
        mpProgramVars["PerFrameCB"]["tex2D_uav"].setUav(pEnvMap->getEnvMap()->getUAV(0));
    }
}

void BRDF_Simulator::setModelVars() {
    mpModelProgramVars["PerFrameCB"]["camPos"] = mpModelScene->getCamera()->getPosition();
    //Material
    mpModelProgramVars["PerFrameCB"]["roughness"] = mRoughness;
    mpModelProgramVars["PerFrameCB"]["metallic"] = metallic;
    mpModelProgramVars["PerFrameCB"]["albedo"] = mAlbedo;
    mpModelProgramVars["PerFrameCB"]["ao"] = ao;
    mpModelProgramVars["PerFrameCB"]["normalizing"] = normalizing;
    //Simulation Type
    mpModelProgramVars["PerFrameCB"]["startBrdf"] = runSimulation;
    mpModelProgramVars["PerFrameCB"]["cookTorrence"] = isCookTorrence;
    mpModelProgramVars["PerFrameCB"]["mySimulation"] = isSimulation;
    mpModelProgramVars["PerFrameCB"]["gSamples"] = jitterInternal;

    
    //Passing textures
    //mpCubeProgramVars["PerFrameCB"]["tex2D_uav"].setTexture
    mpModelProgramVars["PerFrameCB"]["texture2d_0"].setUav(textureVect[0]->getEnvMap()->getUAV(0));
    mpModelProgramVars["PerFrameCB"]["gSampler_0"].setSampler(textureVect[0]->getEnvSampler());
                                                                                        
    mpModelProgramVars["PerFrameCB"]["texture2d_1"].setUav(textureVect[1]->getEnvMap()->getUAV(0));
    mpModelProgramVars["PerFrameCB"]["gSampler_1"].setSampler(textureVect[1]->getEnvSampler());
                                                                                        
    mpModelProgramVars["PerFrameCB"]["texture2d_2"].setUav(textureVect[2]->getEnvMap()->getUAV(0));
    mpModelProgramVars["PerFrameCB"]["gSampler_2"].setSampler(textureVect[2]->getEnvSampler());
                                                                                      
    mpModelProgramVars["PerFrameCB"]["texture2d_3"].setUav( textureVect[3]->getEnvMap()->getUAV(0));
    mpModelProgramVars["PerFrameCB"]["gSampler_3"].setSampler(textureVect[3]->getEnvSampler());
                                                                                      
    mpModelProgramVars["PerFrameCB"]["texture2d_4"].setUav(textureVect[4]->getEnvMap()->getUAV(0));
    mpModelProgramVars["PerFrameCB"]["gSampler_4"].setSampler(textureVect[4]->getEnvSampler());
                                                                                      
    mpModelProgramVars["PerFrameCB"]["texture2d_5"].setUav(textureVect[5]->getEnvMap()->getUAV(0));
    mpModelProgramVars["PerFrameCB"]["gSampler_5"].setSampler(textureVect[5]->getEnvSampler());
                                                                                      
    mpModelProgramVars["PerFrameCB"]["texture2d_6"].setUav(textureVect[6]->getEnvMap()->getUAV(0));
    mpModelProgramVars["PerFrameCB"]["gSampler_6"].setSampler(textureVect[6]->getEnvSampler());
                                                                                      
    mpModelProgramVars["PerFrameCB"]["texture2d_7"].setUav(textureVect[7]->getEnvMap()->getUAV(0));
    mpModelProgramVars["PerFrameCB"]["gSampler_7"].setSampler(textureVect[7]->getEnvSampler());
                                                                                      
    mpModelProgramVars["PerFrameCB"]["texture2d_8"].setUav(textureVect[8]->getEnvMap()->getUAV(0));
    mpModelProgramVars["PerFrameCB"]["gSampler_8"].setSampler(textureVect[8]->getEnvSampler());
                                                                                      
    mpModelProgramVars["PerFrameCB"]["texture2d_9"].setUav(textureVect[9]->getEnvMap()->getUAV(0));
    mpModelProgramVars["PerFrameCB"]["gSampler_9"].setSampler(textureVect[9]->getEnvSampler());
    //pEnvMap->setShaderData(mpVars["PerFrameCB"]["gEnvMap"]);

   // const auto& pEnvMap = mpCubeScene->getEnvMap();
   // mpCubeProgram->addDefine("_USE_ENV_MAP", pEnvMap ? "1" : "0");
    //if (pEnvMap) {
    //    mpModelProgramVars["texture2d_0"].setTexture(pEnvMap->getEnvMap());
    //   // mpModelProgramVars["gSamples"] = jitterInternal;
    //    mpModelProgramVars["gSampler"].setSampler(pEnvMap->getEnvSampler());
    //}


}


//_______________________________________________________________________________________________________________________________________________________________________________________________________

#pragma endregion SHADER VARS FILLING

#pragma region RENEDER TO THE SURFACE SCENE SECTION
/////////////____________RENDER INTO THE SCENE SECTION__________________________/////////////
/*Load surface*/
void BRDF_Simulator::renderSurface() {


    mSceneBuilder = SceneBuilder::create(SceneBuilder::Flags::None);
    SceneBuilder::Node N;
    Falcor::StandardMaterial::SharedPtr Material = StandardMaterial::create("Surface Material", ShadingModel::MetalRough);
    for (int row = 0; row < planSize; row++) {
        for (int col = 0; col < planSize; col++) {
            N.transform[0][3] = float(col) + 1.5;
            N.transform[2][3] = float(row) + 1.5;
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
    mpScene->getMaterialSystem()->setDefaultTextureSampler(mpPointSampler);
}


/*Load the orthographic visualizor quad*/
void BRDF_Simulator::loadOrthoVisualizor(int currLayer) {

    DepthStencilState::Desc tempdsDesc;
    tempdsDesc.setDepthFunc(ComparisonFunc::Less).setDepthEnabled(true);
    mpDebuggingQuadDepthTestDS = DepthStencilState::create(tempdsDesc);

    mpDebuggingQuadSceneBuilder = SceneBuilder::create(SceneBuilder::Flags::None);
    SceneBuilder::Node N;
    float x =  (currLayer * 90.f / float(degOfRotation))* 3.14159265358979323846264338327950288f/180.f;
    //float x = (currLayer * 90.f / float(maxLayer)) * 3.14f / 180.f;
    N.transform[0][3] = float(planSize + 2) / 2.f;
    N.transform[1][3] = float(sin(x)) * 60.f;
    N.transform[2][3] = float(cos(x));// *20.f;// *60.f;
    std::cout << "Value of z: " << N.transform[2][3] << std::endl;
    Falcor::StandardMaterial::SharedPtr Material = StandardMaterial::create("Surface Material", ShadingModel::MetalRough);
    float width = float(planSize) ;
    float height = float(planSize) / 2.f;
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

 
    Falcor::Transform quadTranform = Falcor::Transform::Transform();

    quadTranform.setRotationEuler(float3(1.570800, 0.f, 0.f ));

    N.transform = N.transform * rmcv::mat4_cast(quadTranform.getRotation());
    if (currLayer <= 0) {
        quadTranform.lookAt(float3(N.transform[0][3], N.transform[1][3], N.transform[2][3]), float3(float(planSize + 2) / 2.f, 0.f, float(planSize + 2) / 2.f), float3(0.f, 1.f, 0.f));
    }
    else {

    quadTranform.lookAt(float3(N.transform[0][3], N.transform[1][3], N.transform[2][3]), float3(float(planSize + 2) / 2.f, 0.f, float(planSize + 2) / 2.f), float3(0.f,0.f,1.f));
    }
    N.transform = N.transform * rmcv::mat4_cast(quadTranform.getRotation());
    quadTranform.setRotationEulerDeg(rotateQuad);

    cameraPos = float3(N.transform[0][3], N.transform[1][3], N.transform[2][3]);

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
    orthoCubewireframeDesc.setCullMode(RasterizerState::CullMode::None);
    mpDebuggingQuadWireframeRS = RasterizerState::create(orthoCubewireframeDesc);

    // Depth test
    mpDebuggingQuadGraphicsState->setProgram(mpDebuggingQuadProgram);
    setCamController();
    mpDebuggingQuadScene->getMaterialSystem()->setDefaultTextureSampler(mpLinearSampler);
}

//________________________________________________________________________________________________________________________________
#pragma endregion RENEDER TO THE SURFACE SCENE SECTION

#pragma region LOAD FROM A FILE TO THE MODEL SCENE SECTION
//////////////////_______________LOADING MODEL FROM A FILE TO THE MODEL SCENE_______________/////////////////////////////////////
void BRDF_Simulator::loadModelFromFile(const std::filesystem::path& path, ResourceFormat fboFormat)
{


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


}

bool BRDF_Simulator::loadModel(ResourceFormat fboFormat)
{
    std::filesystem::path path;
    if (openFileDialog(Scene::getFileExtensionFilters(), path))
    {
        loadModelFromFile(path, fboFormat);
        return true;
    }
    return false;
}

//__________________________________________________________________________________________________________________________________

#pragma endregion LOAD FROM A FILE TO THE MODEL SCENE SECTION

#pragma region SET ENVMAP PIPELINE

/*Set the enviroment map pipeline data*/
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
    samplerDesc.setAddressingMode(Sampler::AddressMode::Wrap, Sampler::AddressMode::Clamp, Sampler::AddressMode::Clamp);
    mpCubePointSampler = Sampler::create(samplerDesc);
    // std::filesystem::path path;

    auto pTex = Texture::create2D(envRes, envRes, ResourceFormat::R32Uint, 1, 1, nullptr, ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess);
    pTex->setName("Enviroment Map");

    mpCubeScene->setEnvMap(EnvMap::create(pTex));
    mpCubeScene->getMaterialSystem()->setDefaultTextureSampler(mpCubePointSampler);

}

#pragma endregion SET ENVMAP PIPELINE

#pragma region GUI RENDERING
//////////////////////////____________GUI RENDERING SECTION_______________///////

//Load Surface Pipeline GUI
void BRDF_Simulator::loadSurfaceGUI(Gui::Window& w) {

    Gui::DropdownList cameraDropdown;
    cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::FirstPerson, "First-Person" });
    cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::Orbiter, "Orbiter" });
    cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::SixDOF, "6-DoF" });
    if (w.dropdown("Camera Type", cameraDropdown, (uint32_t&)mCameraType)) setCamController();

    w.separator();



    {
        auto surfaceSettings = w.group("Surface Settings");

        surfaceSettings.var(" Surface Roughness", roughness, 0.f, 1.f);
        surfaceSettings.var(" Surface Size NxN", planSizeTemp, 60, 120, 2);
        if (surfaceSettings.button("Update Surface")) {
            planSize = planSizeTemp;
            renderSurface();
            loadOrthoVisualizor(currLayer);

        }
    }
    {
        auto textureSettings = w.group("Texture Settings");
        textureSettings.var(" Texture Resolution NxN", envResTemp, 150, 300, 1);
        if (textureSettings.button("Update Texture")) {
            envRes = envResTemp;
            createTextures();

        }
    }

    {
        auto simulationSettings = w.group("Simulation Settings");


        simulationSettings.var("Camera Jitter", jitterNum, 2);

        simulationSettings.var("Ray Bounces", bounces, 1);

        simulationSettings.var("Current Layer", currLayer, 1, maxLayer);
    }



    w.separator();

    if (auto statsGroup = w.group("Statistics"))
    {
        const auto& s = mpScene->getSceneStats();
        const double bytesPerTexel = s.materials.textureTexelCount > 0 ? (double)s.materials.textureMemoryInBytes / s.materials.textureTexelCount : 0.0;
        std::ostringstream oss;

        // Frames stats.
        oss << "Frame Stats: " << frames << std::endl
        << std::endl;

        // Geometry stats.
        oss << "Geometry stats:" << std::endl
            << "  Mesh count: " << s.meshCount << std::endl
            << "  Mesh instance count (total): " << s.meshInstanceCount << std::endl
            << "  Mesh instance count (opaque): " << s.meshInstanceOpaqueCount << std::endl
            << "  Mesh instance count (non-opaque): " << (s.meshInstanceCount - s.meshInstanceOpaqueCount) << std::endl
            << "  Transform matrix count: " << s.transformCount << std::endl
            << "  Unique triangle count: " << s.uniqueTriangleCount << std::endl
            << "  Unique vertex count: " << s.uniqueVertexCount << std::endl
            << "  Instanced triangle count: " << s.instancedTriangleCount << std::endl
            << "  Instanced vertex count: " << s.instancedVertexCount << std::endl
            << std::endl;

        // Environment map stats.
        oss << "Environment map:" << std::endl;
        auto mpEnvMap = textureVect[currLayer - 1];//mpCubeScene->getEnvMap();
        if (mpEnvMap)
        {
            oss << "  Resolution: " << textureVect[currLayer - 1]->getEnvMap()->getWidth() << "x" << textureVect[currLayer - 1]->getEnvMap()->getHeight() << std::endl;
        }
        else
        {
            oss << "  N/A" << std::endl;
        }

        oss << std::endl;


        if (statsGroup.button("Print to log")) logInfo("\n" + oss.str());

        statsGroup.text(oss.str());

    }
    w.separator();
    if (w.button("Draw To Textures")) {
        BRDF_Simulation = true;
        mOrthoCam = true;
        currLayerInternal = currLayer;
        jitterInternal = jitterNum;
        bouncesInternal = bounces;
        mpScene->getCamera()->setPosition(cameraPos);
        mpScene->getCamera()->setTarget(float3(float(planSize + 2) / 2.f, 0.f, float(planSize + 2) / 2.f));
        mpScene->getCamera()->setUpVector(float3(0, 1, 0));

    }
    if (w.button("Clear Textures")) {

        createTextures();
    }


    w.separator();

    if (w.button("View Model"))
    {
        //if () {
        mObjectSimulation = true;
        mMicrofacetes = false;
        if (!loadModel(gpFramework->getTargetFbo()->getColorTexture(0)->getFormat())) {
            mObjectSimulation = false;
            mMicrofacetes = true;
        }
        //}
    }

}

//Load Model view Pipeline GUI
void BRDF_Simulator::loadModelGUI(Gui::Window& w) {

    Gui::DropdownList cameraDropdown;
    cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::FirstPerson, "First-Person" });
    cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::Orbiter, "Orbiter" });
    cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::SixDOF, "6-DoF" });

    if (w.dropdown("Camera Type", cameraDropdown, (uint32_t&)mCameraType)) setCamController();

    w.separator();
    if (auto brdf_settings = w.group("BRDF Settings"))
    {
        Gui::DropdownList brdfDropdown;
        brdfDropdown.push_back({ (uint32_t)BRDF_Simulator::BRDF_Type::Cook_Torrance, "Cook Torrance" });
        brdfDropdown.push_back({ (uint32_t)BRDF_Simulator::BRDF_Type::BRDF_Simulation, "BRDF Simulation" });
        if (brdf_settings.dropdown("BRDF Type", brdfDropdown, (uint32_t&)mBRDFType)) {
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



        if (isCookTorrence) {
            brdf_settings.separator();
            brdf_settings.var("Metallic", metallic, 0.f, 1.f);
            brdf_settings.var("Roughness", mRoughness, 0.f, 1.f);
            brdf_settings.var("Albedo", mAlbedo, 0.f, 1.f);
          //  brdf_settings.var("ao", ao, 0.f, 1.f);
        }

        if (isSimulation) {
            brdf_settings.separator();
            brdf_settings.var("Decrease Reflection", normalizing, 1.f);
            brdf_settings.var("Albedo", mAlbedo, 0.f, 1.f);
        }
    }

    w.separator();
    if (auto statsGroup = w.group("Statistics"))
    {
        const auto& s = mpModelScene->getSceneStats();
        const double bytesPerTexel = s.materials.textureTexelCount > 0 ? (double)s.materials.textureMemoryInBytes / s.materials.textureTexelCount : 0.0;
        std::ostringstream oss;

        // Frames stats.
        oss << "Frame Stats: " << frames << std::endl
            << std::endl;

        // Geometry stats.
        oss << "Geometry stats:" << std::endl
            << "  Mesh count: " << s.meshCount << std::endl
            << "  Mesh instance count (total): " << s.meshInstanceCount << std::endl
            << "  Mesh instance count (opaque): " << s.meshInstanceOpaqueCount << std::endl
            << "  Mesh instance count (non-opaque): " << (s.meshInstanceCount - s.meshInstanceOpaqueCount) << std::endl
            << "  Transform matrix count: " << s.transformCount << std::endl
            << "  Unique triangle count: " << s.uniqueTriangleCount << std::endl
            << "  Unique vertex count: " << s.uniqueVertexCount << std::endl
            << "  Instanced triangle count: " << s.instancedTriangleCount << std::endl
            << "  Instanced vertex count: " << s.instancedVertexCount << std::endl
            << std::endl;

        // Environment map stats.
        oss << "Environment map:" << std::endl;
        auto mpEnvMap = textureVect[currLayer - 1];//mpCubeScene->getEnvMap();
        if (mpEnvMap)
        {
            oss << "  Resolution: " << mpEnvMap->getEnvMap()->getWidth() << "x" << mpEnvMap->getEnvMap()->getHeight() << std::endl;
        }
        else
        {
            oss << "  N/A" << std::endl;
        }

        oss << std::endl;


        if (statsGroup.button("Print to log")) logInfo("\n" + oss.str());

        statsGroup.text(oss.str());

    }

    w.separator();
    if (w.button("Start Simulation")) {
        runSimulation = true;
    }

    if (w.button("Stop Simulation")) {
        runSimulation = false;
    }

    if (w.button("View Surface"))
    {
        mMicrofacetes = true;
        mObjectSimulation = false;
        renderSurface();
    }
}

#pragma endregion GUI RENDERING

#pragma region VIEWS RASTERIZATION


/*Rasterize Views*/
void BRDF_Simulator::rasterizeModelView(RenderContext* pRenderContext) {

    mpModelScene->update(pRenderContext, gpFramework->getGlobalClock().getTime());
    mpModelGraphicsState->setDepthStencilState(mpDepthTestDS);
    setModelVars();
   // setEnvMapModelShaderVars();
    mpModelScene->rasterize(pRenderContext, mpModelGraphicsState.get(), mpModelProgramVars.get(), RasterizerState::CullMode::None);
   // mpCubeScene->rasterize(pRenderContext, mpCubeGraphicsState.get(), mpCubeProgramVars.get(), mpRsState, mpRsState);
}

//void BRDF_Simulator::envMapConvert(Falcor::EnvMap& envMap, int currLayer) {
//    textureVect[currLayer] = envMap;
//}
void BRDF_Simulator::rasterizeSurfaceView(RenderContext* pRenderContext) {
    if (currLayerTemp != currLayer) {
        currLayerTemp = currLayer;
        updateEnvMapTexture(false, false, true, currLayer - 1);
        loadOrthoVisualizor(currLayer);
    }

    //if (clearTexture) {
    //    clearTexture = !clearTexture;
    //    updateEnvMapTexture(true, currLayer);

    //   // setEnvMapPipeline();
    //}

    mpScene->update(pRenderContext, gpFramework->getGlobalClock().getTime());
    mpDebuggingQuadScene->update(pRenderContext, gpFramework->getGlobalClock().getTime());

    // Set render state     
    mpDebuggingQuadGraphicsState->setDepthStencilState(mpDebuggingQuadDepthTestDS);
    mpGraphicsState->setDepthStencilState(mpDepthTestDS);
    setSceneVars();

    setOrthoVisualizorVars();
    setEnvMapShaderVars();

    if (mOrthoCam) {
        mpScene->getCamera()->setProjectionMatrix(Falcor::rmcv::ortho(float(-orthCamWidth), float(orthCamWidth), float(-orthCamHeight), float(orthCamHeight), mpScene->getCamera()->getNearPlane(), mpScene->getCamera()->getFarPlane()));
        mpScene->rasterize(pRenderContext, mpGraphicsState.get(), mpProgramVars.get(), RasterizerState::CullMode::Back);
    }
    else {
        mpScene->getCamera()->setProjectionMatrix(Falcor::rmcv::perspective(Falcor::focalLengthToFovY(mpScene->getCamera()->getFocalLength(), mpScene->getCamera()->getFrameHeight()), mpScene->getCamera()->getFrameWidth() / mpScene->getCamera()->getFrameHeight(), mpScene->getCamera()->getNearPlane(), mpScene->getCamera()->getFarPlane()));
        mpScene->rasterize(pRenderContext, mpGraphicsState.get(), mpProgramVars.get(), RasterizerState::CullMode::Back);
    }

    mpDebuggingQuadScene->rasterize(pRenderContext, mpDebuggingQuadGraphicsState.get(), mpDebuggingQuadProgramVars.get(), mpDebuggingQuadWireframeRS, mpDebuggingQuadWireframeRS);

    mpCubeScene->rasterize(pRenderContext, mpCubeGraphicsState.get(), mpCubeProgramVars.get(), mpRsState, mpRsState);
}
#pragma endregion VIEWS RASTERIZATION

#pragma region CAMERA JITTERING
void BRDF_Simulator::jitterCamera() {
    if (jitterInternal <= 1 && BRDF_Simulation) {
        mpScene->getCamera()->setJitter(0, 0);
        //this->switchBool = false;
        this->BRDF_Simulation = false;
        this->mOrthoCam = false;
       // jitterInternal = jitterNum;
        if (currLayerInternal >= 1) {
            continous_simulation = true;
        }
            mpScene->getCamera()->setPosition(cameraPos);
            mpScene->getCamera()->setTarget(float3(float(planSize + 2) / 2.f, 0.f, float(planSize + 2) / 2.f));
            mpScene->getCamera()->setUpVector(float3(0, 1, 0));
        //}
    }

    if (jitterInternal > 1 && BRDF_Simulation && !switchBool) {

        mpScene->getCamera()->setPosition(cameraPos);
        mpScene->getCamera()->setTarget(float3(float(planSize + 2) / 2.f, 0.f, float(planSize + 2) / 2.f));
        mpScene->getCamera()->setUpVector(float3(0, 1, 0));

        float rand01 = get_random();
        float rand02 = get_random();

        //mpScene->getCamera()->setJitter(rand01, rand02);
        mpScene->getCamera()->setPosition(float3(cameraPos.x + rand01, cameraPos.y + rand02, cameraPos.z));
        mpScene->getCamera()->setTarget(float3(float(planSize+ 2) / 2.f, 0.f, float(planSize + 2) / 2.f));
        mpScene->getCamera()->setUpVector(float3(0, 1, 0));
        switchBool = true;
        this->mOrthoCam = true;

        jitterInternal--;
    }
    else if (BRDF_Simulation && switchBool) {

        this->mOrthoCam = true;
        switchBool = false;
        mpScene->getCamera()->setPosition(float3(0.f, 0.f, 0.f));
        mpScene->getCamera()->setTarget(float3(float(planSize+ 2) / 2.f, 0.f, float(planSize + 2) / 2.f));
        mpScene->getCamera()->setUpVector(float3(0, 1, 0));

    }
}
#pragma endregion CAMERA JITTERING

#pragma region CONTINOUS SIMULATION
void BRDF_Simulator::continousSimulation() {
    if (!BRDF_Simulation && continous_simulation && currLayerInternal >= 1 && currLayerInternal <= maxLayer) {
        continous_simulation = false;
        BRDF_Simulation = true;
        mOrthoCam = true;
        currLayerInternal = currLayer;
        jitterInternal = jitterNum;
        bouncesInternal = bounces;
        currLayerInternal -= 1;

        if (currLayerInternal - 1 > -1) {
            //mpCubeScene->getMaterialSystem()->setDefaultTextureSampler(mpCubePointSampler);
            updateEnvMapTexture(false, true, false, currLayerInternal);
            updateEnvMapTexture(false, false, true, currLayerInternal - 1);
            currLayer = currLayerInternal;
            currLayerTemp = currLayerInternal;
            loadOrthoVisualizor(currLayerInternal);
            contSwitchBool = true;
        }
        mpScene->getCamera()->setPosition(cameraPos);
        mpScene->getCamera()->setTarget(float3(float(planSize + 2) / 2.f, 0.f, float(planSize + 2) / 2.f));
        mpScene->getCamera()->setUpVector(float3(0, 1, 0));
    }

    if (!continous_simulation && currLayerInternal < 1) {
        continous_simulation = false;
        BRDF_Simulation = true;
        mOrthoCam = true;
        jitterInternal = jitterNum;
        bouncesInternal = bounces;
        this->switchBool = false;
        updateEnvMapTexture(false, true, false, currLayerInternal);
        updateEnvMapTexture(false, false, true, currLayerInternal);
       // updateEnvMapTexture(false, false, true, currLayerInternal - 1);
        ////currLayerInternal = 2;
        //currLayer = 1;
        //currLayerTemp = 1;
        ////loadOrthoVisualizor(currLayerInternal);

        //mpScene->getCamera()->setJitter(0, 0);
    }
}

#pragma endregion CONTINOUS SIMULATION

void BRDF_Simulator::createTextures() {
    textureVect.clear();
    for (int i = 0; i < maxLayer; i++) {
        Falcor::Texture::SharedPtr pTex = Texture::create2D(envRes, envRes, ResourceFormat::R32Uint, 1, 1, nullptr, ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess);
        pTex->setName("Texture Layer " + std::to_string(i));
        Falcor::EnvMap::SharedPtr pEnv = EnvMap::create(pTex);
        textureVect.push_back(pEnv);
    }

}

void BRDF_Simulator::updateEnvMapTexture(bool clear, bool update, bool get, int currLayer) {
    // = textureVect[currLayer - 1];
    if (clear) {
        Falcor::Texture::SharedPtr pTex = Texture::create2D(envRes, envRes, ResourceFormat::R32Uint, 1, 1, nullptr, ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess);
        pTex->setName("Texture Layer " + std::to_string(currLayer - 1));
        textureVect[currLayer - 1] = EnvMap::create(pTex);
        mpCubeScene->setEnvMap(EnvMap::create(textureVect[currLayer - 1]->getEnvMap()));
       // mpCubeScene->getMaterialSystem()->setDefaultTextureSampler(mpCubePointSampler);
    }
    else if (update) {
       // Falcor::EnvMap::SharedPtr ptex = EnvMap::create();
       
        textureVect[currLayer] = mpCubeScene->getEnvMap();
        Falcor::Texture::SharedPtr pTex = Texture::create2D(envRes, envRes, ResourceFormat::R32Uint, 1, 1, nullptr, ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess);
        pTex->setName("Texture Layer " + std::to_string(currLayer - 1));
        //envMapConvert(*mpCubeScene->getEnvMap(), currLayer);
        mpCubeScene->setEnvMap(EnvMap::create(pTex));
       // mpCubeScene->getMaterialSystem()->setDefaultTextureSampler(mpCubePointSampler);
    }
    else if (get) {

        mpCubeScene->setEnvMap(EnvMap::create(textureVect[currLayer]->getEnvMap()));
       // mpCubeScene->getMaterialSystem()->setDefaultTextureSampler(mpCubePointSampler);
    }

}


void BRDF_Simulator::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "Menu", { 400, 300 }, { 0, 100 });


   // w.separator();

    if (mMicrofacetes) {
        loadSurfaceGUI(w);
    }

    if (mObjectSimulation) {
        loadModelGUI(w);
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
    samplerDesc.setAddressingMode(Sampler::AddressMode::Wrap, Sampler::AddressMode::Clamp, Sampler::AddressMode::Clamp);
    mpPointSampler = Sampler::create(samplerDesc);
    samplerDesc.setFilterMode(Sampler::Filter::Linear, Sampler::Filter::Linear, Sampler::Filter::Linear);
    mpLinearSampler = Sampler::create(samplerDesc);

    createTextures();
   
    resetCamera();
    renderSurface();
    loadOrthoVisualizor(currLayer);
    setEnvMapPipeline();
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
        rasterizeModelView(pRenderContext);
    }
    else if (mpScene && !mObjectSimulation)
    {
        // change the simulation boolean to false.
        jitterCamera();
        rasterizeSurfaceView(pRenderContext);
        if (continous_simulation && !contSwitchBool) {
            timer = 60;
            continousSimulation();
        }
        else if (continous_simulation && contSwitchBool) {
            timer--;
            if (timer <= 0) {
            contSwitchBool = false;
            mpScene->getCamera()->setPosition(float3(0.f, 0.f, 0.f));
            mpScene->getCamera()->setTarget(float3(float(planSize + 2) / 2.f, 0.f, float(planSize + 2) / 2.f));
            mpScene->getCamera()->setUpVector(float3(0, 1, 0));
            }
        }

    }

    frames = gpFramework->getFrameRate().getMsg();

    TextRenderer::render(pRenderContext, mModelString, pTargetFbo, float2(10, 30));

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
    config.windowDesc.title = "BRDF Simulator";
    config.windowDesc.resizableWindow = true;

    Sample::run(config, pRenderer);
    return 0;
}
