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

    enum class TriangleType : uint32_t
    {
        None,
        Quadrilateral,   ///< Quadrilateral Shape
        Dummy,  ///< Dummy Triangle
        Disk,   ///< Cull back-facing primitives
        Cube,
        Sphere,
    };

    struct BRDF_Object {
        Falcor::float3 position;
        BRDF_Simulator::TriangleType shapeType;
        std::string materialName;
        Falcor::ShadingModel shadingModel;
    };


    void onLoad(RenderContext* pRenderContext) override;
    void onFrameRender(RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo) override;
    void onResizeSwapChain(uint32_t width, uint32_t height) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onGuiRender(Gui* pGui) override;

private:
    void renderGeometry();
    void resetCamera();
    void setModelString(double loadTime);
    void setCamController();
    void updateGrid();
    void addShaderLib();
    void removeOutGridObj();
    bool isExist();

    bool mUseTriLinearFiltering = true;
    Sampler::SharedPtr mpPointSampler = nullptr;
    Sampler::SharedPtr mpLinearSampler = nullptr;
    GraphicsProgram::SharedPtr mpProgram = nullptr;
    GraphicsVars::SharedPtr mpProgramVars = nullptr;
    GraphicsState::SharedPtr mpGraphicsState = nullptr;

    bool mDrawWireframe = false;
    bool mUseOriginalTangents = false;
    bool mDontMergeMaterials = false;
    bool mIsGeometry = false;
    bool mOrthoCam = false;
    bool mTogglePers = false;
    Scene::CameraControllerType mCameraType = Scene::CameraControllerType::Orbiter;

    Scene::SharedPtr mpScene;
    SceneBuilder::SharedPtr mSceneBuilder ;
    RasterizerState::SharedPtr mpWireframeRS = nullptr;
    RasterizerState::CullMode mCullMode = RasterizerState::CullMode::Back;

    DepthStencilState::SharedPtr mpNoDepthDS = nullptr;
    DepthStencilState::SharedPtr mpDepthTestDS = nullptr;

    std::vector<BRDF_Object> brdf_Objects;
    Falcor::int2 gridSizeTemp = Falcor::int2(1);
    Falcor::int2 gridSize = Falcor::int2(1);
    Falcor::float3 shapePosition = Falcor::float3(0);
    Falcor::rmcv::mat4 storeProjMat = Falcor::rmcv::mat4(1.f);
    BRDF_Simulator::TriangleType mTriangleType = BRDF_Simulator::TriangleType::None;
    std::string mModelString;
};
