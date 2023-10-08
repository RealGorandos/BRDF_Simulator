add_falcor_executable(BRDF_Simulator)

target_sources(BRDF_Simulator PRIVATE
    BRDF_Simulator.cpp
    BRDF_Simulator.h
    Surface.3d.hlsl
    Cook_Torrence.brdf.hlsl
    BRDF_Simulation.hlsl
    EnvMap.3d.hlsl
    Model.3d.hlsl
    Visualizor.3d.hlsl
    Simulation.lib.hlsl
)

target_copy_shaders(BRDF_Simulator Samples/BRDF_Simulator)
target_copy_data_folder(BRDF_Simulator Samples/BRDF_Simulator)

target_source_group(BRDF_Simulator "Samples")
