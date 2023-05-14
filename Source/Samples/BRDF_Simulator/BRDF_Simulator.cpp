#include "BRDF_Simulator.h"
#include "Utils/UI/TextRenderer.h"
#include "Utils/Math/FalcorMath.h"
#include <math.h>
#include <random>
#include <chrono> 
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
float get_random(unsigned long int jitterInternal)
{
    //seed *= 10;
    static std::default_random_engine e;// { static_cast<long unsigned int>(seed * 10)};
    e.seed(jitterInternal);
    static std::uniform_real_distribution<float> dis(-1.0f, 1.0f); // rage 0 - 1
    return dis(e);
}
/////______________________________________________________________________________________________________________
#pragma endregion HELPER METHODS

#pragma region SHADER VARS FILLING
/////////////___FILLING THE SHADERS VARIABLES SECTION__///////////////////////////////////
void BRDF_Simulator::setEnvMapShaderVars() {

    const auto& pEnvMap = textureVect[currLayer - 1];
    mpCubeProgram->addDefine("_USE_ENV_MAP", pEnvMap ? "1" : "0");
    if (pEnvMap) {
        mpCubeProgramVars["PerFrameCB"]["tex2D_uav"].setTexture(textureVect[currLayer - 1]->getEnvMap());
        mpCubeProgramVars["PerFrameCB"]["gSamples"] = maxJitter;//* bouncesInternal;
        mpCubeProgramVars["PerFrameCB"]["envSampler"].setSampler(samplerVect[currLayer - 1]);
    }

    rmcv::mat4 world = rmcv::translate(mpScene->getCamera()->getPosition());

    mpCubeProgramVars["PerFrameCB"]["gWorld"] = world;
    mpCubeProgramVars["PerFrameCB"]["gScale"] = 1.f;
    mpCubeProgramVars["PerFrameCB"]["gViewMat"] = mpScene->getCamera()->getViewMatrix();
    mpCubeProgramVars["PerFrameCB"]["gProjMat"] = mpCubeScene->getCamera()->getProjMatrix();
    mpCubeProgramVars["PerFrameCB"]["camRes"] = float(planSize);
}

void BRDF_Simulator::setEnvMapModelShaderVars() {

    const auto& pEnvMap = textureVect[currLayer - 1];
    mpCubeProgram->addDefine("_USE_ENV_MAP", pEnvMap ? "1" : "0");
    if (pEnvMap) {
        mpCubeProgramVars["PerFrameCB"]["tex2D_uav"].setTexture(textureVect[currLayer - 1]->getEnvMap());
        mpCubeProgramVars["PerFrameCB"]["gSamples"] = maxJitter;// *bouncesInternal;
        mpCubeProgramVars["PerFrameCB"]["envSampler"].setSampler(samplerVect[currLayer - 1]);
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
    mpProgramVars["PerFrameCB"]["BRDF_Simulation"] = BRDF_Simulation;//&& switchBool;
    mpProgramVars["PerFrameCB"]["roughness"] = roughness;
    mpProgramVars["PerFrameCB"]["surfaceSize"] = planSize;
    mpProgramVars["PerFrameCB"]["bounces"] = bounces;
    mpProgramVars["PerFrameCB"]["c_dir"] = mpScene->getCamera()->getTarget() - mpScene->getCamera()->getPosition();
    const auto& pEnvMap = textureVect[currLayer - 1];
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
    mpModelProgramVars["PerFrameCB"]["gSamples"] = jitterInternalStatic;// *bouncesInternal;

    mpModelProgramVars["PerFrameCB"]["camRes"] = float(planSize);
    
    //Passing textures
    //mpCubeProgramVars["PerFrameCB"]["tex2D_uav"].setTexture
    mpModelProgramVars["PerFrameCB"]["texture2d_0"].setUav(textureVect[0]->getEnvMap()->getUAV(0));
    //mpModelProgramVars["PerFrameCB"]["gSampler_0"].setSampler(textureVect[0]->getEnvSampler());
                                                                                        
    mpModelProgramVars["PerFrameCB"]["texture2d_1"].setUav(textureVect[1]->getEnvMap()->getUAV(0));
    //mpModelProgramVars["PerFrameCB"]["gSampler_1"].setSampler(textureVect[1]->getEnvSampler());
                                                                                        
    mpModelProgramVars["PerFrameCB"]["texture2d_2"].setUav(textureVect[2]->getEnvMap()->getUAV(0));
   // mpModelProgramVars["PerFrameCB"]["gSampler_2"].setSampler(textureVect[2]->getEnvSampler());
                                                                                      
    mpModelProgramVars["PerFrameCB"]["texture2d_3"].setUav( textureVect[3]->getEnvMap()->getUAV(0));
   // mpModelProgramVars["PerFrameCB"]["gSampler_3"].setSampler(textureVect[3]->getEnvSampler());
                                                                                      
    mpModelProgramVars["PerFrameCB"]["texture2d_4"].setUav(textureVect[4]->getEnvMap()->getUAV(0));
   // mpModelProgramVars["PerFrameCB"]["gSampler_4"].setSampler(textureVect[4]->getEnvSampler());
                                                                                      
    mpModelProgramVars["PerFrameCB"]["texture2d_5"].setUav(textureVect[5]->getEnvMap()->getUAV(0));
   // mpModelProgramVars["PerFrameCB"]["gSampler_5"].setSampler(textureVect[5]->getEnvSampler());
                                                                                      
    mpModelProgramVars["PerFrameCB"]["texture2d_6"].setUav(textureVect[6]->getEnvMap()->getUAV(0));
   // mpModelProgramVars["PerFrameCB"]["gSampler_6"].setSampler(textureVect[6]->getEnvSampler());
                                                                                      
    mpModelProgramVars["PerFrameCB"]["texture2d_7"].setUav(textureVect[7]->getEnvMap()->getUAV(0));
   // mpModelProgramVars["PerFrameCB"]["gSampler_7"].setSampler(textureVect[7]->getEnvSampler());
                                                                                      
    mpModelProgramVars["PerFrameCB"]["texture2d_8"].setUav(textureVect[8]->getEnvMap()->getUAV(0));
   // mpModelProgramVars["PerFrameCB"]["gSampler_8"].setSampler(textureVect[8]->getEnvSampler());
                                                                                      
    mpModelProgramVars["PerFrameCB"]["texture2d_9"].setUav(textureVect[9]->getEnvMap()->getUAV(0));
   // mpModelProgramVars["PerFrameCB"]["gSampler_9"].setSampler(textureVect[9]->getEnvSampler());
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

    //switch (mSurfRes) {
    //case  BRDF_Simulator::ProgramRes::_64x64:
    //    planSize = 64;
    //    break;
    //case  BRDF_Simulator::ProgramRes::_128x128:
    //    planSize = 128;
    //    break;
    //case  BRDF_Simulator::ProgramRes::_256x256:
    //    planSize = 256;
    //    break;
    //case  BRDF_Simulator::ProgramRes::_512x512:
    //    planSize = 512;
    //    break;
    //default:
    //    FALCOR_UNREACHABLE();
    //}
    
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
        desc.addShaderLibrary("Samples/BRDF_Simulator/BRDF_Simulator.3d.hlsl").vsEntry("vsMain").gsEntry("gsMain").psEntry("psMain");
        desc.addTypeConformances(mpScene->getTypeConformances());

        mpProgram = GraphicsProgram::create(desc, mpScene->getSceneDefines());
    }

    mpProgramVars = GraphicsVars::create(mpProgram->getReflector());
    mpGraphicsState->setProgram(mpProgram);

    setCamController();
    //mpScene->getMaterialSystem()->setDefaultTextureSampler(mpPointSampler);
}


/*Load the orthographic visualizor quad*/
void BRDF_Simulator::loadOrthoVisualizor(int currLayer) {

    DepthStencilState::Desc tempdsDesc;
    tempdsDesc.setDepthFunc(ComparisonFunc::Less).setDepthEnabled(true);
    mpDebuggingQuadDepthTestDS = DepthStencilState::create(tempdsDesc);

    mpDebuggingQuadSceneBuilder = SceneBuilder::create(SceneBuilder::Flags::None);
    SceneBuilder::Node N;
    float deg =  -(currLayer * 90.f / float(degOfRotation))* 3.14159265358979323846264338327950288f/180.f - M_PI;
    //float x = (currLayer * 90.f / float(maxLayer)) * 3.14f / 180.f;
    const float z = 50.f;
    const float y = 0.f;
    N.transform[0][3] = float(planSize + 2) / 2.f; // x
    N.transform[1][3] = z * float(sin(deg)); //z * float(sin(deg)) ;// float(cos(deg)) * y - float(sin(deg)) * z;   // y
    N.transform[2][3] = z * float(cos(deg)) + float(planSize + 2) / 2.f; // z* float(cos(deg)) + 20.f; //float(sin(deg)) * y + float(cos(deg)) * z;    // z
   
    Falcor::StandardMaterial::SharedPtr Material = StandardMaterial::create("Surface Material", ShadingModel::MetalRough);
    float width = float(planSize) ;
    float height = float(planSize);
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

    quadTranform.setRotationEuler(float3(-deg - M_PI_2, 0.f, 0.f ));
    /*
    N.transform = N.transform * rmcv::mat4_cast(quadTranform.getRotation());
    if (currLayer <= 0) {
        quadTranform.lookAt(float3(N.transform[0][3], N.transform[1][3], N.transform[2][3]), float3(float(planSize + 2) / 2.f, 0.f, float(planSize + 2) / 2.f), float3(0.f, 1.f, 0.f));
    }
    else {

    quadTranform.lookAt(float3(N.transform[0][3], N.transform[1][3], N.transform[2][3]), float3(float(planSize + 2) / 2.f, 0.f, float(planSize + 2) / 2.f), float3(0.f,0.f,1.f));
    }*/
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
  //  mpDebuggingQuadScene->getMaterialSystem()->setDefaultTextureSampler(mpLinearSampler);
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
    std::filesystem::path path;// = "TestScenes/Lucy.obj";
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






    {
        auto surfaceSettings = w.group("Surface Settings");

        surfaceSettings.var(" Surface Size NxN", planSizeTemp, 60, 120, 2);
        if (surfaceSettings.button("Apply Size")) {
            planSize = planSizeTemp;
            renderSurface();
            loadOrthoVisualizor(currLayer);

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


        simulationSettings.var("Camera Jitter", jitterNum, 1, maxJitter);

        simulationSettings.var("Ray Bounces", bounces, 1);

        simulationSettings.var("Current Layer", currLayer, 1, maxLayer);
        simulationSettings.tooltip("Enter a value instead of dragging.", true);
    }

    Gui::DropdownList cameraDropdown;
    cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::FirstPerson, "First-Person" });
    cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::Orbiter, "Orbiter" });
    cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::SixDOF, "6-DoF" });
    if (w.dropdown("Camera Type", cameraDropdown, (uint32_t&)mCameraType)) setCamController();

    w.separator();
    {
 
       // Gui::Window x(w.gui(), "Statistics", { 400, 300 }, { 0, 100 });



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
        //if () {
        mObjectSimulation = true;
        mMicrofacetes = false;
        if (!loadModel(gpFramework->getTargetFbo()->getColorTexture(0)->getFormat())) {
            mObjectSimulation = false;
            mMicrofacetes = true;
        }
        //}
    }


    if (w.button("Run Test", true)) {
        runTest = true;
        
    }

    if (runTest) {
        Gui::Window x(w, "Test Window", { 400, 500 }, { 100, 100 });
        const auto& s = mpScene->getSceneStats();
        const double bytesPerTexel = s.materials.textureTexelCount > 0 ? (double)s.materials.textureMemoryInBytes / s.materials.textureTexelCount : 0.0;

        //int totallPassed = 0;
        if (auto texResTest = x.group("Texture Resolution Test:", true))
        {
            if (iterateOnce == 1) {
                
                int passedCnt = 0;
                int currRes = 32;
                //oss << " " << std::endl;
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
        //oss.clear();
        if (auto LoadModelTest = x.group("Load Model Test:", true))
        {
            if (openOnce == 1) {
                int passedCnt = 0;
              
                Falcor::Scene::SceneStats a;
                Falcor::Scene::SceneStats b;
                modelOSS << "\tTest loading a Model with a lot of triangles: " << std::endl;
           
                loadModelFromFile("TestScenes/Lucy.obj" , gpFramework->getTargetFbo()->getColorTexture(0)->getFormat());
                modelOSS << "\t\t\tModel path: Falcor/media/TestScenes/Lucy.obj" << std::endl;
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

                modelOSS << "\tTest loading a model with fewer triangles: " << std::endl;
                mpModelScene = nullptr;
                
                loadModelFromFile("TestScenes/Bunny.obj", gpFramework->getTargetFbo()->getColorTexture(0)->getFormat());
                modelOSS << "\t\t\tModel path: Falcor/media/TestScenes/Bunny.obj" << std::endl;
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
                modelOSS << "\t\tLucy has more triangles than Bunny --> " ;
                if (a.uniqueTriangleCount > b.uniqueTriangleCount) {
                    modelOSS << "PASSED" << std::endl;
                    passedCnt++;
                }
                else {
                    modelOSS << "FAILED" << std::endl;
                }

                modelOSS << "\t\tLucy has more vertices than Bunny --> ";
                if (a.uniqueVertexCount > b.uniqueVertexCount) {
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

        if (auto UpdateSurfaceTest = x.group("Update Surface Test:", true))
        {
            if (updateOnce == 1) {
                int temp = planSize;
                Falcor::Scene::SceneStats a;
                Falcor::Scene::SceneStats b;
                planSize = 60;
                int passedCnt = 0;

                surfaceOSS << "\tTest loading minimum surface size -->";
                renderSurface();
                a = mpScene->getSceneStats();
                int expectedTriangles = 60 * 60 * 2;
                if (a.uniqueTriangleCount == expectedTriangles) {
                    surfaceOSS << "PASSED" << std::endl;
                    passedCnt++;
                }
                else {
                    surfaceOSS << "FAILD" << std::endl;
                }

                planSize = 120;

                surfaceOSS << "\tTest loading maximum surface size -->";
                renderSurface();
                b = mpScene->getSceneStats();
                expectedTriangles = 120 * 120 * 2;
                if (b.uniqueTriangleCount == expectedTriangles) {
                    surfaceOSS << "PASSED" << std::endl;
                    passedCnt++;
                }
                else {
                    surfaceOSS << "FAILD" << std::endl;
                }

                surfaceOSS << std::endl;
                surfaceOSS << "\tTesting the two surfaces: " << std::endl;
                surfaceOSS << "\t\tMaximum surface has more triangles than minimum -->";
                if (b.uniqueTriangleCount > a.uniqueTriangleCount) {
                    surfaceOSS << "PASSED" << std::endl;
                    passedCnt++;
                }
                else {
                    surfaceOSS << "FAILD" << std::endl;
                }

  
                surfaceOSS << "\t\tMaximum surface has more vertices than minimum -->";
                if (b.uniqueVertexCount > a.uniqueVertexCount) {
                    surfaceOSS << "PASSED" << std::endl;
                    passedCnt++;
                }
                else {
                    surfaceOSS << "FAILD" << std::endl;
                }

                planSize = temp;
                renderSurface();

                updateOnce--;
                totallPassed += passedCnt;
                surfaceOSS << std::endl;
                surfaceOSS << "\tResult: " << std::endl;
                surfaceOSS << "\t\t" << std::to_string(passedCnt) << " out of " << std::to_string(4) << " Passed" << std::endl;
            }
            UpdateSurfaceTest.text(surfaceOSS.str());
        }
        x.separator();

        x.text( "\tFinal Result:\n");
        x.text("\t\t" + std::to_string(totallPassed) + " out of " + std::to_string(83) + " Passed \n" );
        //oss.clear();
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
        loadOrthoVisualizor(currLayer);
    }



    mpScene->update(pRenderContext, gpFramework->getGlobalClock().getTime());
    mpDebuggingQuadScene->update(pRenderContext, gpFramework->getGlobalClock().getTime());

    // Set render state     
    mpDebuggingQuadGraphicsState->setDepthStencilState(mpDebuggingQuadDepthTestDS);
    mpGraphicsState->setDepthStencilState(mpDepthTestDS);
    setSceneVars();

    setOrthoVisualizorVars();
    //std::cout << "We are here!!!" << e
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
    if (jitterInternal <= 0 && BRDF_Simulation) {

        //this->switchBool = false;
        this->BRDF_Simulation = false;
        this->mOrthoCam = true;
       // jitterInternal = jitterNum;
        if (currLayerInternal >= 1) {
            currLayerInternal -= 1;
            continous_simulation = true;
        }
            mpScene->getCamera()->setPosition(cameraPos);
            mpScene->getCamera()->setTarget(float3(float(planSize + 2) / 2.f, 0.f, float(planSize + 2) / 2.f));
            mpScene->getCamera()->setUpVector(float3(0, 1, 0));
        //}
    }

    if (jitterInternal > 0 && BRDF_Simulation) {


        seedIncOdd += 2;
        seedIncEven += 3;

        float rand01 = get_random(jitterInternal + seedIncEven + int(ceil(cameraPos.x)) + int(ceil(cameraPos.y)) + int(ceil(cameraPos.z)) + currLayerInternal);
        float rand02 = get_random(jitterInternal + seedIncOdd + int(ceil(cameraPos.x)) + int(ceil(cameraPos.y)) + int(ceil(cameraPos.z)) + currLayerInternal);
        
        mpScene->getCamera()->setPosition(float3(cameraPos.x + rand01, cameraPos.y + rand02, cameraPos.z));
        mpScene->getCamera()->setTarget(float3(float(planSize+ 2) / 2.f, 0.f, float(planSize + 2) / 2.f));
        mpScene->getCamera()->setUpVector(float3(0, 1, 0));
       // switchBool = true;
        this->mOrthoCam = true;
        continous_simulation = false;
        jitterInternal--;
    }

}
#pragma endregion CAMERA JITTERING

#pragma region CONTINOUS SIMULATION
void BRDF_Simulator::continousSimulation() {
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
        loadOrthoVisualizor(currLayerInternal);
        

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
       // currLayerInternal = 1;
        //currLayer = 1;// currLayerInternal;
        //currLayerTemp = currLayer;
       // loadOrthoVisualizor(currLayer);
        mpScene->getCamera()->setPosition(cameraPos);
        mpScene->getCamera()->setTarget(float3(float(planSize + 2) / 2.f, 0.f, float(planSize + 2) / 2.f));
        mpScene->getCamera()->setUpVector(float3(0, 1, 0));
    }
}

#pragma endregion CONTINOUS SIMULATION

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
   // Gui::MainMenu m(pGui);
   // Gui::MainMenu n(pGui);
   // Gui::Menu n(pGui, "Something");


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
    samplerDesc.setFilterMode(Sampler::Filter::Linear, Sampler::Filter::Linear, Sampler::Filter::Linear);
    samplerDesc.setAddressingMode(Sampler::AddressMode::Clamp, Sampler::AddressMode::Clamp, Sampler::AddressMode::Clamp);
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
        continousSimulation();

        


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
