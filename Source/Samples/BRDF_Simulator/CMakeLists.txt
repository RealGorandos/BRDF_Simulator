add_falcor_executable(BRDF_Simulator)

target_sources(BRDF_Simulator PRIVATE
    BRDF_Simulator.cpp
    BRDF_Simulator.h
)

target_copy_shaders(BRDF_Simulator Samples/BRDF_Simulator)

target_source_group(BRDF_Simulator "Samples")
