#include "BRDF_Simulator.h"
#include "Utils/UI/TextRenderer.h"
#include "Utils/Math/FalcorMath.h"
#include <math.h>
#include <random>
#include <chrono> 
#include <Utils/StringUtils.h>


#pragma region HELPER METHODS SECTION

/*Gets a random number*/
float get_random(unsigned long int jitterInternal)
{

    static std::default_random_engine e;
    e.seed(jitterInternal);
    static std::uniform_real_distribution<float> dis(-0.5f, 0.5f);
    return dis(e);
}

/*Update and clear textures*/
void BRDF_Simulator::createTextures() {
    textureVect.clear();
    samplerVect.clear();
    switch (mTexRes) {
    case  BRDF_Simulator::ProgramRes::_32x32:
        envRes = 32;
        break;
    case  BRDF_Simulator::ProgramRes::_64x64:
        envRes = 64;
        break;
    case  BRDF_Simulator::ProgramRes::_128x128:
        envRes = 128;
        break;
    case  BRDF_Simulator::ProgramRes::_256x256:
        envRes = 256;
        break;
    case  BRDF_Simulator::ProgramRes::_512x512:
        envRes = 512;
        break;
    default:
        FALCOR_UNREACHABLE();
    }

    for (int i = 0; i < maxLayer; i++) {
        Falcor::Texture::SharedPtr pTex = Texture::create2D(envRes, envRes, ResourceFormat::R32Uint, 1, 1, nullptr, ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess);
        pTex->setName("Texture Layer " + std::to_string(i));
        Falcor::EnvMap::SharedPtr pEnv = EnvMap::create(pTex);
        textureVect.push_back(pEnv);

        Sampler::Desc samplerDesc;
        samplerDesc.setFilterMode(Sampler::Filter::Point, Sampler::Filter::Point, Sampler::Filter::Point);
        samplerDesc.setAddressingMode(Sampler::AddressMode::Wrap, Sampler::AddressMode::Clamp, Sampler::AddressMode::Clamp);
        Falcor::Sampler::SharedPtr pSampler = Sampler::create(samplerDesc);

        samplerVect.push_back(pSampler);
    }

}
#pragma endregion HELPER METHODS

#pragma region SHADER VARS FILLING
void BRDF_Simulator::setEnvMapShaderVars() {

    const auto& pEnvMap = textureVect[currLayer - 1];
    mpEnvMapProgram->addDefine("_USE_ENV_MAP", pEnvMap ? "1" : "0");
    if (pEnvMap) {
        mpEnvMapProgramVars["PerFrameCB"]["tex2D_uav"].setTexture(textureVect[currLayer - 1]->getEnvMap());
        mpEnvMapProgramVars["PerFrameCB"]["gSamples"] = jitterInternalStatic;
        mpEnvMapProgramVars["PerFrameCB"]["envSampler"].setSampler(samplerVect[currLayer - 1]);
    }

    rmcv::mat4 world = rmcv::translate(mpScene->getCamera()->getPosition());

    mpEnvMapProgramVars["PerFrameCB"]["gWorld"] = world;
    mpEnvMapProgramVars["PerFrameCB"]["gScale"] = 1.f;
    mpEnvMapProgramVars["PerFrameCB"]["gViewMat"] = mpScene->getCamera()->getViewMatrix();
    mpEnvMapProgramVars["PerFrameCB"]["gProjMat"] = mpEnvMapScene->getCamera()->getProjMatrix();
    mpEnvMapProgramVars["PerFrameCB"]["camRes"] = float(planSize);
}



void BRDF_Simulator::setOrthoVisualizorVars() {

    rmcv::mat4 world = rmcv::translate(float3(0.f));
    mpVisualizorProgramVars["PerFrameCB"]["gWorld"] = world;
    mpVisualizorProgramVars["PerFrameCB"]["gViewMat"] = mpScene->getCamera()->getViewMatrix();
    mpVisualizorProgramVars["PerFrameCB"]["gProjMat"] = mpScene->getCamera()->getProjMatrix();
    mpVisualizorProgramVars["PerFrameCB"]["gSize"] = planSize;
    mpVisualizorProgramVars["PerFrameCB"]["deg"] = deg;
    mpVisualizorProgramVars["PerFrameCB"]["scaleFact"] = scaleFactor;
}

void BRDF_Simulator::setSceneVars() {
    mpProgramVars["PerFrameCB"]["BRDF_Simulation"] = BRDF_Simulation;
    mpProgramVars["PerFrameCB"]["roughness"] = roughness;
    mpProgramVars["PerFrameCB"]["surfaceSize"] = planSize;
    mpProgramVars["PerFrameCB"]["bounces"] = bounces;

    mpProgramVars["PerFrameCB"]["c_dir"] = normalize(mpScene->getCamera()->getTarget() - mpScene->getCamera()->getPosition());

        mpProgramVars["PerFrameCB"]["tex2D_uav"].setUav(textureVect[currLayer - 1]->getEnvMap()->getUAV(0));
 
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
    mpModelProgramVars["PerFrameCB"]["gSamples"] = jitterInternalStatic;

    mpModelProgramVars["PerFrameCB"]["camRes"] = float(planSize);
    
    //Passing textures
    mpModelProgramVars["PerFrameCB"]["texture2d_0"].setTexture(textureVect[0]->getEnvMap());
    mpModelProgramVars["PerFrameCB"]["gSampler_0"].setSampler(textureVect[0]->getEnvSampler());
                                                                                        
    mpModelProgramVars["PerFrameCB"]["texture2d_1"].setTexture(textureVect[1]->getEnvMap());
    mpModelProgramVars["PerFrameCB"]["gSampler_1"].setSampler(textureVect[1]->getEnvSampler());
                                                                                        
    mpModelProgramVars["PerFrameCB"]["texture2d_2"].setTexture(textureVect[2]->getEnvMap());
    mpModelProgramVars["PerFrameCB"]["gSampler_2"].setSampler(textureVect[2]->getEnvSampler());
                                                                                      
    mpModelProgramVars["PerFrameCB"]["texture2d_3"].setTexture( textureVect[3]->getEnvMap());
    mpModelProgramVars["PerFrameCB"]["gSampler_3"].setSampler(textureVect[3]->getEnvSampler());
                                                                                      
    mpModelProgramVars["PerFrameCB"]["texture2d_4"].setTexture(textureVect[4]->getEnvMap());
    mpModelProgramVars["PerFrameCB"]["gSampler_4"].setSampler(textureVect[4]->getEnvSampler());
                                                                                      
    mpModelProgramVars["PerFrameCB"]["texture2d_5"].setTexture(textureVect[5]->getEnvMap());
    mpModelProgramVars["PerFrameCB"]["gSampler_5"].setSampler(textureVect[5]->getEnvSampler());
                                                                                      
    mpModelProgramVars["PerFrameCB"]["texture2d_6"].setTexture(textureVect[6]->getEnvMap());
    mpModelProgramVars["PerFrameCB"]["gSampler_6"].setSampler(textureVect[6]->getEnvSampler());
                                                                                      
    mpModelProgramVars["PerFrameCB"]["texture2d_7"].setTexture(textureVect[7]->getEnvMap());
    mpModelProgramVars["PerFrameCB"]["gSampler_7"].setSampler(textureVect[7]->getEnvSampler());
                                                                                      
    mpModelProgramVars["PerFrameCB"]["texture2d_8"].setTexture(textureVect[8]->getEnvMap());
    mpModelProgramVars["PerFrameCB"]["gSampler_8"].setSampler(textureVect[8]->getEnvSampler());
                                                                                      
    mpModelProgramVars["PerFrameCB"]["texture2d_9"].setTexture(textureVect[9]->getEnvMap());
    mpModelProgramVars["PerFrameCB"]["gSampler_9"].setSampler(textureVect[9]->getEnvSampler());

    mpModelProgramVars["PerFrameCB"]["texture2d_10"].setTexture(textureVect[10]->getEnvMap());
    mpModelProgramVars["PerFrameCB"]["gSampler_10"].setSampler(textureVect[10]->getEnvSampler());

    mpModelProgramVars["PerFrameCB"]["texture2d_11"].setTexture(textureVect[11]->getEnvMap());
    mpModelProgramVars["PerFrameCB"]["gSampler_11"].setSampler(textureVect[11]->getEnvSampler());

    mpModelProgramVars["PerFrameCB"]["texture2d_12"].setTexture(textureVect[12]->getEnvMap());
    mpModelProgramVars["PerFrameCB"]["gSampler_12"].setSampler(textureVect[12]->getEnvSampler());

    mpModelProgramVars["PerFrameCB"]["texture2d_13"].setTexture(textureVect[13]->getEnvMap());
    mpModelProgramVars["PerFrameCB"]["gSampler_13"].setSampler(textureVect[13]->getEnvSampler());

    mpModelProgramVars["PerFrameCB"]["texture2d_14"].setTexture(textureVect[14]->getEnvMap());
    mpModelProgramVars["PerFrameCB"]["gSampler_14"].setSampler(textureVect[14]->getEnvSampler());




}



#pragma endregion SHADER VARS FILLING

#pragma region RENEDER TO THE SURFACE SCENE SECTION
/*Load surface*/
void BRDF_Simulator::renderSurface() {


    
    mSceneBuilder = SceneBuilder::create(SceneBuilder::Flags::None);
    SceneBuilder::Node N;
    Falcor::StandardMaterial::SharedPtr Material = StandardMaterial::create("Surface Material", ShadingModel::MetalRough);
    Falcor::TriangleMesh::SharedPtr TriangleMesh = TriangleMesh::createQuad(float2(1.f));
    for (int row = 0; row < planSize; row++) {
        for (int col = 0; col < planSize; col++) {
            N.transform[0][3] = float(col) + 1.5;
            N.transform[2][3] = float(row) + 1.5;
            mSceneBuilder->addMeshInstance(mSceneBuilder->addNode(N), mSceneBuilder->addTriangleMesh(TriangleMesh, Material));
        }
    }
    mpScene = mSceneBuilder->getScene();

    {
        Program::Desc desc;
        desc.addShaderModules(mpScene->getShaderModules());
        desc.addShaderLibrary("Samples/BRDF_Simulator/Surface.3d.hlsl").vsEntry("vsMain").gsEntry("gsMain").psEntry("psMain");
        desc.addTypeConformances(mpScene->getTypeConformances());

        mpProgram = GraphicsProgram::create(desc, mpScene->getSceneDefines());
    }

    mpProgramVars = GraphicsVars::create(mpProgram->getReflector());
    mpGraphicsState->setProgram(mpProgram);

    mpScene->setCameraSpeed(120.f);
    setCamController();
}

/*Calculates the direction*/
void BRDF_Simulator::updateVisualizorTransformMat() {

     deg = -(currLayer * 90.f / float(degOfRotation)) * 3.14159265358979323846264338327950288f / 180.f - M_PI;
    const float z = float(planSize + 2) / 2 + 10.f;
    const float y = 0.f;
    N.transform[0][3] = float(planSize + 2) / 2.f; // x
    N.transform[1][3] = z * float(sin(deg));  // y
    N.transform[2][3] = z * float(cos(deg)) + float(planSize + 2) / 2;  // z

    orthCamWidth = float(planSize) / 2.f;
    orthCamHeight = float(planSize) / 2.f;

    Falcor::Transform quadTranform = Falcor::Transform::Transform();
    quadTranform.setRotationEuler(float3(-deg - M_PI_2, 0.f, 0.f));

    N.transform = N.transform * rmcv::mat4_cast(quadTranform.getRotation());

    cameraPos = float3(N.transform[0][3], N.transform[1][3], N.transform[2][3]);

}

/*Load the orthographic visualizor quad*/
void BRDF_Simulator::loadOrthoVisualizor(int currLayer) {

    DepthStencilState::Desc tempdsDesc;
    tempdsDesc.setDepthFunc(ComparisonFunc::Less).setDepthEnabled(true);
    mpVisualizorDepthTestDS = DepthStencilState::create(tempdsDesc);

    mpVisualizorSceneBuilder = SceneBuilder::create(SceneBuilder::Flags::None);
    float width = float(planSize);
    float height = float(planSize);
   
    Falcor::StandardMaterial::SharedPtr Material = StandardMaterial::create("Surface Material", ShadingModel::MetalRough);

    Falcor::TriangleMesh::SharedPtr quadTemp = TriangleMesh::createQuad(float2(width, height));

    quadTemp->addVertex( float3(-(0.5f * width), width, -(0.5f * height)), float3(0.f, 0.f, 0.f), float2( 0.f, 0.f ));
    quadTemp->addVertex({ (0.5f * width), width, -(0.5f * height) }, float3(0.f, 0.f, 0.f), { 1.f, 0.f });
    quadTemp->addVertex({ -(0.5f * width), width,  (0.5f * height) }, float3(0.f, 0.f, 0.f), { 0.f, 1.f });
    quadTemp->addVertex({ (0.5f * width), width,  (0.5f * height) }, float3(0.f, 0.f, 0.f), { 1.f, 1.f });

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

 
    updateVisualizorTransformMat();

    mpVisualizorSceneBuilder->addMeshInstance(mpVisualizorSceneBuilder->addNode(N), mpVisualizorSceneBuilder->addTriangleMesh(quadTemp, Material));


    mpVisualizorScene = mpVisualizorSceneBuilder->getScene();

    {
        Program::Desc desc;
        desc.addShaderModules(mpVisualizorScene->getShaderModules());
        desc.addShaderLibrary("Samples/BRDF_Simulator/Visualizor.3d.hlsl").vsEntry("vsMain").psEntry("psMain");
        desc.addTypeConformances(mpVisualizorScene->getTypeConformances());

        mpVisualizorProgram = GraphicsProgram::create(desc, mpVisualizorScene->getSceneDefines());
    }

    mpVisualizorProgramVars = GraphicsVars::create(mpVisualizorProgram->getReflector());




    // Create rasterizer state
    RasterizerState::Desc orthoCubewireframeDesc;
    orthoCubewireframeDesc.setFillMode(RasterizerState::FillMode::Wireframe);
    orthoCubewireframeDesc.setCullMode(RasterizerState::CullMode::None);
    mpVisualizorWireframeRS = RasterizerState::create(orthoCubewireframeDesc);

    // Depth test
    mpVisualizorGraphicsState->setProgram(mpVisualizorProgram);
    setCamController();
}

#pragma endregion RENEDER TO THE SURFACE SCENE SECTION

#pragma region LOAD FROM A FILE TO THE MODEL SCENE SECTION
void BRDF_Simulator::loadModelFromFile(const std::filesystem::path& path, ResourceFormat fboFormat)
{


    SceneBuilder::Flags flags = SceneBuilder::Flags::None;
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

    mpModelScene->getMaterialSystem()->setDefaultTextureSampler(mpPointSampler);
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


#pragma endregion LOAD FROM A FILE TO THE MODEL SCENE SECTION

#pragma region SET ENVMAP PIPELINE

/*Set the enviroment map pipeline data*/
void BRDF_Simulator::setEnvMapPipeline() {


    mpEnvMapScene = Scene::create("cube.obj");

    mpEnvMapProgram = GraphicsProgram::createFromFile("Samples/BRDF_Simulator/EnvMap.3d.hlsl", "vsMain", "psMain");
    mpEnvMapProgram->addDefines(mpEnvMapScene->getSceneDefines());
    mpEnvMapProgramVars = GraphicsVars::create(mpEnvMapProgram->getReflector());

    // Create state
    mpEnvMapGraphicsState = GraphicsState::create();
    BlendState::Desc blendDesc;
    for (uint32_t i = 1; i < Fbo::getMaxColorTargetCount(); i++) blendDesc.setRenderTargetWriteMask(i, false, false, false, false);
    blendDesc.setIndependentBlend(true);
    mpEnvMapGraphicsState->setBlendState(BlendState::create(blendDesc));

    // Create the rasterizer state
    RasterizerState::Desc rastDesc;
    rastDesc.setCullMode(RasterizerState::CullMode::None).setDepthClamp(true);
    mpRsState = RasterizerState::create(rastDesc);

    DepthStencilState::Desc dsDesc;
    dsDesc.setDepthWriteMask(false).setDepthFunc(DepthStencilState::Func::LessEqual);
    mpEnvMapGraphicsState->setDepthStencilState(DepthStencilState::create(dsDesc));
    mpEnvMapGraphicsState->setProgram(mpEnvMapProgram);

    Sampler::Desc samplerDesc;
    samplerDesc.setFilterMode(Sampler::Filter::Point, Sampler::Filter::Point, Sampler::Filter::Point);
    samplerDesc.setAddressingMode(Sampler::AddressMode::Wrap, Sampler::AddressMode::Clamp, Sampler::AddressMode::Clamp);
    mpEnvMapPointSampler = Sampler::create(samplerDesc);

    auto pTex = Texture::create2D(envRes, envRes, ResourceFormat::R32Uint, 1, 1, nullptr, ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess);
    pTex->setName("Enviroment Map");

    mpEnvMapScene->setEnvMap(EnvMap::create(pTex));
    mpEnvMapScene->getMaterialSystem()->setDefaultTextureSampler(mpEnvMapPointSampler);

}

#pragma endregion SET ENVMAP PIPELINE

#pragma region GUI RENDERING

//Load Surface Pipeline GUI
void BRDF_Simulator::loadSurfaceGUI(Gui::Window& w) {






    {
        auto surfaceSettings = w.group("Surface Settings");

        surfaceSettings.var(" Surface Size NxN", planSizeTemp, 120, maxPlaneSize, 2);
        if (surfaceSettings.button("Apply Size")) {
            
            planSize = planSizeTemp;
            scaleFactor = planSize / float(maxPlaneSize);
            createTextures();
            renderSurface();
            updateVisualizorTransformMat();

        }
        surfaceSettings.tooltip("Click \"Apply Size\" to apply the surface size changes.", true);

        surfaceSettings.var(" Surface Roughness", roughness, 0.f, 1.f);


    }

    Gui::DropdownList textureResolutions;
    textureResolutions.push_back({ (uint32_t)BRDF_Simulator::ProgramRes::_32x32, "32x32" });
    textureResolutions.push_back({ (uint32_t)BRDF_Simulator::ProgramRes::_64x64, "64x64" });
    textureResolutions.push_back({ (uint32_t)BRDF_Simulator::ProgramRes::_128x128, "128x128" });
    textureResolutions.push_back({ (uint32_t)BRDF_Simulator::ProgramRes::_256x256, "256x256" });
    textureResolutions.push_back({ (uint32_t)BRDF_Simulator::ProgramRes::_512x512, "512x512" });

    {
        auto textureSettings = w.group("Texture Settings");



        if (textureSettings.dropdown("Texture Resolution", textureResolutions, (uint32_t&)mTexRes)) createTextures();
    }

    {
        auto simulationSettings = w.group("Simulation Settings");


        simulationSettings.var("Camera Jitter", jitterNum, 1);

        simulationSettings.var("Ray Bounces", bounces, 1);

        simulationSettings.slider("Current Layer", currLayer, 1, maxLayer);

    }

    Gui::DropdownList cameraDropdown;
    cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::FirstPerson, "First-Person" });
    cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::Orbiter, "Orbiter" });
    cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::SixDOF, "6-DoF" });
    if (w.dropdown("Camera Type", cameraDropdown, (uint32_t&)mCameraType)) setCamController();

    w.separator();
    {
 



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
            auto mpEnvMap = textureVect[currLayer - 1];
            if (mpEnvMap)
            {
                oss << "  Resolution: " << textureVect[currLayer - 1]->getEnvMap()->getWidth() << "x" << textureVect[currLayer - 1]->getEnvMap()->getHeight() << std::endl;
            }
            else
            {
                oss << "  N/A" << std::endl;
            }

            oss << std::endl;

            oss << "Rays Stats:" << std::endl;
            oss << "  Rays Count: " << float(planSize) * float(planSize) * jitterNum * currLayer * 2  << std::endl;
            if (statsGroup.button("Print to log")) logInfo("\n" + oss.str());

            statsGroup.text(oss.str());

        }
    }
    w.separator();
    if (w.button("Draw To Textures")) {
        createTextures();
        seedIncEven = 1;
        seedIncOdd = 1;

        BRDF_Simulation = true;
        mOrthoCam = true;
        if (currLayer != maxLayer) {
            currLayer = maxLayer;
            updateVisualizorTransformMat();
        }
        currLayerInternal = currLayer;
        jitterInternal = jitterNum;
        jitterInternalStatic = jitterNum;
        bouncesInternal = bounces;
        mpScene->getCamera()->setPosition(cameraPos);
        mpScene->getCamera()->setTarget(float3(float(planSize + 2) / 2.f, 0.f, float(planSize + 2) / 2.f));
        mpScene->getCamera()->setUpVector(float3(0, 1, 0));

    }
    if (w.button("Clear Textures", true)) { createTextures();}

    if (w.button("View Model", true))
    {
        mObjectSimulation = true;
        mMicrofacetes = false;
        if (!loadModel(gpFramework->getTargetFbo()->getColorTexture(0)->getFormat())) {
            mObjectSimulation = false;
            mMicrofacetes = true;
        }
        
    }


    if (w.button("Run Test", true)) {
        runTest = true;
        
    }

    if (runTest) {
        Gui::Window x(w, "Test Window", { 400, 500 }, { 100, 100 });
        const auto& s = mpScene->getSceneStats();
        const double bytesPerTexel = s.materials.textureTexelCount > 0 ? (double)s.materials.textureMemoryInBytes / s.materials.textureTexelCount : 0.0;

        if (auto texResTest = x.group("Texture Resolution Test:", true))
        {
            if (iterateOnce == 1) {
                
                int passedCnt = 0;
                int currRes = 32;
                for (auto resType : textureResolutions) {
                    (uint32_t&)mTexRes = resType.value;
                    createTextures();
                    textureOSS << "\tTesting Resolution " << std::to_string(currRes) << "x" << std::to_string(currRes) << std::endl;
                    for (int i = 0; i < 15; i++) {
                        int width = textureVect[i]->getEnvMap()->getWidth();
                        int height = textureVect[i]->getEnvMap()->getHeight();
                        textureOSS << "\t\tTesting if resolution of layer  " << std::to_string(i + 1) << " is " << std::to_string(currRes) << "x" << std::to_string(currRes) << "    ---->   ";
                        if (width == currRes && height == currRes) {
                            textureOSS << "PASSED" << std::endl;
                            passedCnt++;
                        }
                        else {
                            textureOSS << "FAILED" << std::endl;
                        }

                    }
                    currRes *= 2;
                    textureOSS << std::endl;
                }
                totallPassed += passedCnt;
                textureOSS << "\tResult: " <<  std::endl;
                textureOSS << "\t\t" << std::to_string(passedCnt) << " out of " << std::to_string(15 * textureResolutions.size()) << " Passed" << std::endl;
                iterateOnce--;
            }
            texResTest.text(textureOSS.str());
        }
        if (auto LoadModelTest = x.group("Load Model Test:", true))
        {
            if (openOnce == 1) {
                int passedCnt = 0;
              
                Falcor::Scene::SceneStats a;
                Falcor::Scene::SceneStats b;
                modelOSS << "\tTest loading a Model with few triangles: " << std::endl;
           
                loadModelFromFile("Lucy.obj" , gpFramework->getTargetFbo()->getColorTexture(0)->getFormat());
                modelOSS << "\t\t\tModel: Lucy.obj" << std::endl;
                modelOSS << "\t\t\t\tOpening Model --> ";
                if (mpModelScene) {
                    modelOSS << " PASSED" << std::endl;
                        a = mpModelScene->getSceneStats();
                        passedCnt++;
                }
                else {
                    modelOSS << " FAILED" << std::endl;
                }
           
                modelOSS << std::endl;

                modelOSS << "\tTest loading a model with a lot triangles: " << std::endl;
                mpModelScene = nullptr;
                
                loadModelFromFile("Bunny.obj", gpFramework->getTargetFbo()->getColorTexture(0)->getFormat());
                modelOSS << "\t\t\tModel: Bunny.obj" << std::endl;
                modelOSS << "\t\t\t\tOpening Model --> ";
                if (mpModelScene) {
                    modelOSS << " PASSED" << std::endl;
                    b = mpModelScene->getSceneStats();
                    passedCnt++;
                }
                else {
                    modelOSS << " FAILED" << std::endl;
                }

                mpModelScene = nullptr;

                modelOSS << std::endl;

                modelOSS << "\tTesting the two models: " << std::endl;
                modelOSS << "\t\tBunny.obj has more triangles than Lucy.obj --> " ;
                if (a.uniqueTriangleCount < b.uniqueTriangleCount) {
                    modelOSS << "PASSED" << std::endl;
                    passedCnt++;
                }
                else {
                    modelOSS << "FAILED" << std::endl;
                }

                modelOSS << "\t\tBunny.obj has more vertices than Lucy.obj --> ";
                if (a.uniqueVertexCount < b.uniqueVertexCount) {
                    modelOSS << "PASSED" << std::endl;
                    passedCnt++;
                }
                else {
                    modelOSS << "FAILED" << std::endl;
                }
                openOnce--;
                totallPassed += passedCnt;
                modelOSS << std::endl;
                modelOSS << "\tResult: " << std::endl;
                modelOSS << "\t\t" << std::to_string(passedCnt) << " out of " << std::to_string(4) << " Passed" << std::endl;
            }
                LoadModelTest.text(modelOSS.str());
        }

        x.separator();

        x.text( "\tFinal Result:\n");
        x.text("\t\t" + std::to_string(totallPassed) + " out of " + std::to_string(79) + " Passed \n" );
        if (x.button("Exit")) {
            runTest = false;
        }

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
    }
}

#pragma endregion GUI RENDERING

#pragma region VIEWS RASTERIZATION


/*Rasterize Views*/
void BRDF_Simulator::rasterizeModelView(RenderContext* pRenderContext) {

    mpModelScene->update(pRenderContext, gpFramework->getGlobalClock().getTime());
    mpModelGraphicsState->setDepthStencilState(mpDepthTestDS);
    setModelVars();
    mpModelScene->rasterize(pRenderContext, mpModelGraphicsState.get(), mpModelProgramVars.get(), RasterizerState::CullMode::None);
}


void BRDF_Simulator::rasterizeSurfaceView(RenderContext* pRenderContext) {
    if (currLayerTemp != currLayer) {
       // scaleFactor = currLayer
        currLayerTemp = currLayer;
        updateVisualizorTransformMat();
    }



    mpScene->update(pRenderContext, gpFramework->getGlobalClock().getTime());
    mpVisualizorScene->update(pRenderContext, gpFramework->getGlobalClock().getTime());

    // Set render state     
    mpVisualizorGraphicsState->setDepthStencilState(mpVisualizorDepthTestDS);
    mpGraphicsState->setDepthStencilState(mpDepthTestDS);
    setSceneVars();

    setOrthoVisualizorVars();
    setEnvMapShaderVars();

    if (mOrthoCam) {
        mpScene->getCamera()->setProjectionMatrix(Falcor::rmcv::ortho(float(-orthCamWidth), float(orthCamWidth), float(-orthCamHeight), float(orthCamHeight), mpScene->getCamera()->getNearPlane(), mpScene->getCamera()->getFarPlane()));
        mpScene->rasterize(pRenderContext, mpGraphicsState.get(), mpProgramVars.get(), RasterizerState::CullMode::None);
    }
    else {
        mpScene->getCamera()->setProjectionMatrix(Falcor::rmcv::perspective(Falcor::focalLengthToFovY(mpScene->getCamera()->getFocalLength(), mpScene->getCamera()->getFrameHeight()), mpScene->getCamera()->getFrameWidth() / mpScene->getCamera()->getFrameHeight(), mpScene->getCamera()->getNearPlane(), mpScene->getCamera()->getFarPlane()));
        mpScene->rasterize(pRenderContext, mpGraphicsState.get(), mpProgramVars.get(), RasterizerState::CullMode::None);
    }

    mpVisualizorScene->rasterize(pRenderContext, mpVisualizorGraphicsState.get(), mpVisualizorProgramVars.get(), mpVisualizorWireframeRS, mpVisualizorWireframeRS);

    mpEnvMapScene->rasterize(pRenderContext, mpEnvMapGraphicsState.get(), mpEnvMapProgramVars.get(), mpRsState, mpRsState);
}
#pragma endregion VIEWS RASTERIZATION

#pragma region CAMERA JITTERING
/*Jitter camera for sampling*/
void BRDF_Simulator::cameraJitter() {
    if (jitterInternal <= 0 && BRDF_Simulation) {

        this->BRDF_Simulation = false;
        this->mOrthoCam = true;
        if (currLayerInternal >= 1) {
            currLayerInternal -= 1;
            continous_simulation = true;
        }
            mpScene->getCamera()->setPosition(cameraPos);
            mpScene->getCamera()->setTarget(float3(float(planSize + 2) / 2.f, 0.f, float(planSize + 2) / 2.f));
            mpScene->getCamera()->setUpVector(float3(0, 1, 0));
        
    }

    if (jitterInternal > 0 && BRDF_Simulation) {



        seedIncOdd += 2;
        seedIncEven += 3;

        float rand01 = get_random(jitterInternal + seedIncEven + int(ceil(cameraPos.x)) + int(ceil(cameraPos.y)) + int(ceil(cameraPos.z)) + currLayerInternal);
        float rand02 = get_random(jitterInternal + seedIncOdd + int(ceil(cameraPos.x)) + int(ceil(cameraPos.y)) + int(ceil(cameraPos.z)) + currLayerInternal);
       if (currLayer == 15) {
            mpScene->getCamera()->setPosition(float3(cameraPos[0] + rand01 , cameraPos[1] , cameraPos[2] + rand02));
            mpScene->getCamera()->setTarget(float3(float(planSize+ 2) / 2.f + rand01, 0.f , float(planSize + 2) / 2.f) + rand02);
        }
       else if (currLayer <= 4) {
           mpScene->getCamera()->setPosition(float3(cameraPos[0] + rand01, cameraPos[1] + rand02, cameraPos[2]));
           mpScene->getCamera()->setTarget(float3(float(planSize) / 2.f + rand01 + 0.5f, rand02, float(planSize + 2) / 2.f  + 0.5f));
       }
        else {
            mpScene->getCamera()->setPosition(float3(cameraPos[0] + rand01, cameraPos[1] + rand02, cameraPos[2]));
            mpScene->getCamera()->setTarget(float3(float(planSize) / 2.f + rand01 + 0.5f, 0.f, float(planSize + 2) / 2.f  +rand02 + 0.5f));
        }


        mpScene->getCamera()->setUpVector(float3(0, 1, 0));
        

        this->mOrthoCam = true;
        continous_simulation = false;
        jitterInternal--;
    }

}




/*Prepare Variables for the next layer jittering*/
void BRDF_Simulator::updateJitter() {
    if (!BRDF_Simulation && continous_simulation && currLayerInternal >= 1 && currLayerInternal <= maxLayer) {
        seedIncEven = 1;
        seedIncOdd = 1;
        continous_simulation = false;
        BRDF_Simulation = true;
        mOrthoCam = true;
        currLayer = currLayerInternal;
        jitterInternal = jitterNum;
        bouncesInternal = bounces;

        currLayer = currLayerInternal;
        currLayerTemp = currLayerInternal;
        updateVisualizorTransformMat();


        mpScene->getCamera()->setPosition(cameraPos);
        mpScene->getCamera()->setTarget(float3(float(planSize + 2) / 2.f, 0.f, float(planSize + 2) / 2.f));
        mpScene->getCamera()->setUpVector(float3(0, 1, 0));
    }

    if (continous_simulation && currLayerInternal <= 0) {
        continous_simulation = false;
        BRDF_Simulation = false;
        mOrthoCam = false;
        jitterInternal = jitterNum;
        bouncesInternal = bounces;

        mpScene->getCamera()->setPosition(cameraPos);
        mpScene->getCamera()->setTarget(float3(float(planSize + 2) / 2.f, 0.f, float(planSize + 2) / 2.f));
        mpScene->getCamera()->setUpVector(float3(0, 1, 0));
    }
}
#pragma endregion CAMERA JITTERING






void BRDF_Simulator::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "Menu", { 400, 300 }, { 0, 100 });



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
    mpVisualizorGraphicsState = GraphicsState::create();
    mpModelGraphicsState = GraphicsState::create();

    // Depth test
    DepthStencilState::Desc dsDesc;
    dsDesc.setDepthFunc(ComparisonFunc::Less).setDepthEnabled(true);
    mpDepthTestDS = DepthStencilState::create(dsDesc);

    Sampler::Desc samplerDesc;
    samplerDesc.setFilterMode(Sampler::Filter::Linear, Sampler::Filter::Linear, Sampler::Filter::Linear);
    samplerDesc.setAddressingMode(Sampler::AddressMode::Clamp, Sampler::AddressMode::Clamp, Sampler::AddressMode::Clamp);
    mpPointSampler = Sampler::create(samplerDesc);


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
    mpVisualizorGraphicsState->setFbo(pTargetFbo);
    mpModelGraphicsState->setFbo(pTargetFbo);
    mpGraphicsState->setFbo(pTargetFbo);
    mpEnvMapGraphicsState->setFbo(pTargetFbo);

    if (mpModelScene && !mMicrofacetes) {
        rasterizeModelView(pRenderContext);
    }
    else if (mpScene && !mObjectSimulation)
    {
        cameraJitter();
        rasterizeSurfaceView(pRenderContext);
        updateJitter();
    }

    frames = gpFramework->getFrameRate().getMsg();

    TextRenderer::render(pRenderContext, "", pTargetFbo, float2(10, 30));

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
    else if (mpScene && !mObjectSimulation && !BRDF_Simulation) {
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
    else if (mpScene && !mObjectSimulation && !BRDF_Simulation && !continous_simulation) {
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
