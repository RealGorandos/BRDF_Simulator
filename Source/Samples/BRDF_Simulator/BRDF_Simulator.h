#pragma once
#include "Falcor.h"
#include <vector>
using namespace Falcor;

class BRDF_Simulator : public IRenderer
{
public:

    enum class BRDF_Type
    {
        BRDF_Simulation,
        Cook_Torrance
    };

    //Textures resolutions
    enum class ProgramRes
    {
        _32x32,
        _64x64,
        _128x128,
        _256x256,
        _512x512
    };

    void onLoad(RenderContext* pRenderContext) override;
    void onFrameRender(RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo) override;
    void onResizeSwapChain(uint32_t width, uint32_t height) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onGuiRender(Gui* pGui) override;

private:
    //Loading model from a file
    bool loadModel(ResourceFormat fboFormat);
    void loadModelFromFile(const std::filesystem::path& path, ResourceFormat fboFormat);
    void loadFromFileTesting(const std::filesystem::path& path, ResourceFormat fboFormat);

    //Render the microfacts surface
    void renderSurface();

    //Camera functions
    void resetCamera();
    void setCamController();

    //Render the enviroment map and setup the pipeline
    void setEnvMapPipeline();

    //Passing variables to the pipelines
    void setEnvMapShaderVars();
    void setModelVars();
    void setOrthoVisualizorVars();
    void setSceneVars();

    //load the wireframe visualizator
    void loadOrthoVisualizor(int currLayer);
    void updateVisualizorTransformMat();

    //Surface pipeline gui
    void loadSurfaceGUI(Gui::Window& w);
    void loadModelGUI(Gui::Window& w);

    //Rasterize pipelines
    void rasterizeSurfaceView(RenderContext* pRenderContext);
    void rasterizeModelView(RenderContext* pRenderContext);

    //Jitter camera
    void cameraJitter();
    void updateJitter();

    //Update EnvMap texture function
    void createTextures();

    //Run Testing
    void startTest(Gui::Window& w, Gui::DropdownList textureResolutions);

    Sampler::SharedPtr mpPointSampler = nullptr;
    DepthStencilState::SharedPtr mpDepthTestDS = nullptr;

    //Main scene PIPELINE
    Scene::SharedPtr mpScene = nullptr;
    GraphicsProgram::SharedPtr mpProgram = nullptr;
    GraphicsVars::SharedPtr mpProgramVars = nullptr;
    GraphicsState::SharedPtr mpGraphicsState = nullptr;
    SceneBuilder::SharedPtr mSceneBuilder = nullptr;

    //Model PIPELINE
    Scene::SharedPtr mpModelScene;
    GraphicsProgram::SharedPtr mpModelProgram = nullptr;
    GraphicsVars::SharedPtr mpModelProgramVars = nullptr;
    GraphicsState::SharedPtr mpModelGraphicsState = nullptr;
    Scene::SharedPtr mpTestModelScene = nullptr;

    //CubeBox scene PIPELINE
    Scene::SharedPtr mpEnvMapScene;
    GraphicsProgram::SharedPtr mpEnvMapProgram = nullptr;
    GraphicsVars::SharedPtr mpEnvMapProgramVars = nullptr;
    GraphicsState::SharedPtr mpEnvMapGraphicsState = nullptr;
    RasterizerState::SharedPtr mpRsState = nullptr;
    Sampler::SharedPtr mpEnvMapPointSampler = nullptr;
    RasterizerState::CullMode mCubeCullMode = RasterizerState::CullMode::None;

    //Orthographic Camera simulation PIPELINE
    GraphicsProgram::SharedPtr mpVisualizorProgram = nullptr;
    GraphicsVars::SharedPtr mpVisualizorProgramVars = nullptr;
    GraphicsState::SharedPtr mpVisualizorGraphicsState = nullptr;
    SceneBuilder::SharedPtr mpVisualizorSceneBuilder;
    Scene::SharedPtr mpVisualizorScene;
    RasterizerState::SharedPtr mpVisualizorWireframeRS = nullptr;
    DepthStencilState::SharedPtr mpVisualizorDepthTestDS = nullptr;
    Falcor::rmcv::mat4 visualizorTranformMat;
    SceneBuilder::Node N;

   //Dropdown lists
    Scene::CameraControllerType mCameraType = Scene::CameraControllerType::Orbiter;
    BRDF_Simulator::BRDF_Type mBRDFType = BRDF_Simulator::BRDF_Type::Cook_Torrance;
    BRDF_Simulator::ProgramRes mTexRes = BRDF_Simulator::ProgramRes::_64x64;

    //Surface variables
    int planSize = 512; //Surface Of Microfacets size
    int planSizeTemp = planSize;
    int maxPlaneSize = 512;
    float roughness = 0.f;

    //Ortho Camera variables
    float orthCamWidth = 2;
    float orthCamHeight = 1;
    bool mOrthoCam = false;

    //Simulation Variables
    unsigned long int seedIncEven = 1;
    unsigned long int seedIncOdd = 1;
    int maxJitter = 100;
    int jitterNum = 2;
    int bounces = 2;
    int jitterInternal = 2;
    int jitterInternalStatic = 2;
    int bouncesInternal = 2;
    int bouncesInternalStatic = 2;

    //Buttons variables
    bool BRDF_Simulation = false;
    bool clearTexture = false;
    bool continous_simulation = false;

    //Wireframe Visualizator variables
    float3 rotateQuad = float3(0.f, 0.f, 90.f);
    int degOfRotation = 15;
    int maxLayer = 15;
    float3 cameraPos = float3(0.f);
    float deg = 0.f;
    float scaleFactor = 1.f;

    //Layers variables
    int currLayer = 1;
    int currLayerTemp = 1;
    int currLayerInternal = 1;

    //Envmap textures and samplers
    std::vector<Falcor::EnvMap::SharedPtr> textureVect;
    std::vector<Falcor::Sampler::SharedPtr> samplerVect;

    //Env Map resolution
    int envRes = 64;
    int envResTemp = envRes;

    //Model view switching variables
    bool mMicrofacetes = true;
    bool mObjectSimulation = false;

    //BRDF method boolean
    bool isCookTorrence = true;

    //Our BRDF boolean
    bool isSimulation = false;

    //Run brdf simulation
    bool runSimulation = false;

    //BRDFs variables
    float metallic = 0.f;
    float mRoughness = 0.f;
    float3 mAlbedo = float3(0.f, 0.f ,0.f);
    float ao = float(0.f);
    float3 lightIntensity = Falcor::float3(0.f);
    float normalizing = 0.f;

    //Testing Variables
    bool runTest = false;
    std::ostringstream modelOSS;
    std::ostringstream textureOSS;
    int updateOnce = 1;
    int openOnce = 1;
    int iterateOnce = 1;
    int totallPassed = 0;
    float kEpsilon = 0.000001f;
    std::string frames;
};
