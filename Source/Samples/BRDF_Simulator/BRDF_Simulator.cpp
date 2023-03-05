#include "BRDF_Simulator.h"
#include "Utils/UI/TextRenderer.h"
#include "Utils/Math/FalcorMath.h"

Falcor::Camera::SharedPtr constructCamera(const Falcor::Camera::SharedPtr refCamera);

void BRDF_Simulator::setModelString(double loadTime)
{
    FALCOR_ASSERT(mpScene != nullptr);

    mModelString = "Loading took " + std::to_string(loadTime) + " seconds.\n";
    //mModelString += "Model has " + std::to_string(pModel->getVertexCount()) + " vertices, ";
    //mModelString += std::to_string(pModel->getIndexCount()) + " indices, ";
    //mModelString += std::to_string(pModel->getPrimitiveCount()) + " primitives, ";
    mModelString += std::to_string(mpScene->getMeshCount()) + " meshes, ";
    mModelString += std::to_string(mpScene->getGeometryInstanceCount()) + " instances, ";
    mModelString += std::to_string(mpScene->getMaterialCount()) + " materials, ";
    //mModelString += std::to_string(pModel->getTextureCount()) + " textures, ";
    //mModelString += std::to_string(pModel->getBufferCount()) + " buffers.\n";
}

void BRDF_Simulator::addShaderLib() {
    {
        Program::Desc desc;
        desc.addShaderModules(mpScene->getShaderModules());
        desc.addShaderLibrary("Samples/BRDF_Simulator/BRDF_Simulator.3d.hlsl").vsEntry("vsMain").psEntry("psMain");
        desc.addTypeConformances(mpScene->getTypeConformances());

        mpProgram = GraphicsProgram::create(desc, mpScene->getSceneDefines());
    }

    mpProgramVars = GraphicsVars::create(mpProgram->getReflector());
    mpGraphicsState->setProgram(mpProgram);

    setCamController();

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
        addShaderLib();
        timer.update();
        setModelString(timer.delta());
}


void BRDF_Simulator::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "BRDF Simulator Demo", { 400, 300 }, { 0, 100 });

    {
        auto gridSettings = w.group("Plan Settings");
        gridSettings.var("Plan Size", planSizeTemp, 10);
        if (gridSettings.button("Update Plan")) {
            planSize = planSizeTemp;
            renderSurface();
        }
    }


    {
        auto loadGroup = w.group("Load Options");
        loadGroup.checkbox("Use Original Tangents", mUseOriginalTangents);
        loadGroup.tooltip("If this is unchecked, we will ignore the tangents that were loaded from the model and calculate them internally. Check this box if you'd like to use the original tangents");
        loadGroup.checkbox("Don't Merge Materials", mDontMergeMaterials);
        loadGroup.tooltip("Don't merge materials that have the same properties. Use this option to preserve the original material names.");
    }

    w.separator();
    w.checkbox("Wireframe", mDrawWireframe);

    if (mDrawWireframe == false)
    {
        Gui::DropdownList cullList;
        cullList.push_back({ (uint32_t)RasterizerState::CullMode::None, "No Culling" });
        cullList.push_back({ (uint32_t)RasterizerState::CullMode::Back, "Backface Culling" });
        cullList.push_back({ (uint32_t)RasterizerState::CullMode::Front, "Frontface Culling" });
        w.dropdown("Cull Mode", cullList, (uint32_t&)mCullMode);
    }

    Gui::DropdownList cameraDropdown;
    cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::FirstPerson, "First-Person" });
    cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::Orbiter, "Orbiter" });
    cameraDropdown.push_back({ (uint32_t)Scene::CameraControllerType::SixDOF, "6-DoF" });

    if (w.dropdown("Camera Type", cameraDropdown, (uint32_t&)mCameraType)) setCamController();

    if (w.checkbox("Orthographic View", mOrthoCam)) { resetCamera(); }

    //if (mpScene) mpScene->renderUI(w);
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
    samplerDesc.setFilterMode(Sampler::Filter::Linear, Sampler::Filter::Linear, Sampler::Filter::Linear);
    mpLinearSampler = Sampler::create(samplerDesc);

    resetCamera();
    renderSurface();
}

void BRDF_Simulator::onFrameRender(RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo)
{
    const float4 clearColor(0.4f, 0.4f, 0.4f, 1);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);
    mpGraphicsState->setFbo(pTargetFbo);

    if (mpScene)
    {
        mpScene->update(pRenderContext, gpFramework->getGlobalClock().getTime());

        // Set render state
        if (mDrawWireframe)
        {
            mpGraphicsState->setDepthStencilState(mpNoDepthDS);
            mpProgramVars["PerFrameCB"]["gConstColor"] = true;

            mpScene->rasterize(pRenderContext, mpGraphicsState.get(), mpProgramVars.get(), mpWireframeRS, mpWireframeRS);
        }
        else
        {
            mpGraphicsState->setDepthStencilState(mpDepthTestDS);
            mpProgramVars["PerFrameCB"]["gConstColor"] = false;

            mpScene->rasterize(pRenderContext, mpGraphicsState.get(), mpProgramVars.get(), mCullMode);
        }
    }

    if (mOrthoCam) {
        mpScene->getCamera()->setProjectionMatrix(Falcor::rmcv::ortho(-4.0f * mpScene->getCamera()->getAspectRatio(), 4.0f * mpScene->getCamera()->getAspectRatio(), -4.0f , 4.0f, mpScene->getCamera()->getNearPlane(), mpScene->getCamera()->getFarPlane()));
    }
    else {
        mpScene->getCamera()->setProjectionMatrix(Falcor::rmcv::perspective(Falcor::focalLengthToFovY(mpScene->getCamera()->getFocalLength(), mpScene->getCamera()->getFrameHeight()), mpScene->getCamera()->getFrameWidth()/ mpScene->getCamera()->getFrameHeight(), mpScene->getCamera()->getNearPlane(), mpScene->getCamera()->getFarPlane()));

    }
    TextRenderer::render(pRenderContext, mModelString, pTargetFbo, float2(10, 30));
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
