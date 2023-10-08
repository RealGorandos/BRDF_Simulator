# Efficient Simulation of Bidirectional Reflectance Distribution Functions

My thesis is about efficient simulation of bidirectional reflectance distribution functions (BRDFs). This repository contains my thesis program as well as the paperwork.

---

# - BRDF_Simulator

This is a solution that introduces a method for efficiently simulating Bidirectional Reflectance Distribution Functions (BRDFs) by applying ray tracing on a generated surface of microfacets and storing the outgoing rays in sky-maps. It is then utilized and compared with existing models, such as the Cook-Torrance model, in a few sample scenes. This solution is powered by Nvidia Falcor Framework, which is a real-time rendering framework supporting DirectX 12 and Vulkan.


<p align="center">
  <img src="https://github.com/RealGorandos/BRDF_Simulator/assets/84874186/b9bc8596-b234-4f29-a420-be7860337e32" />
</p>

<p align="center">
  <img src="https://github.com/RealGorandos/BRDF_Simulator/assets/84874186/96304de9-bddb-478f-9448-6b017e1d5e11" />
</p>


---

# - Falcor 5.2

Click [here](https://github.com/NVIDIAGameWorks/Falcor) to read about falcor framework and its prerequisties.

## - Citation

If you use Falcor in a research project leading to a publication, please cite the project.
The BibTex entry is

```bibtex
@Misc{Kallweit22,
   author =      {Simon Kallweit and Petrik Clarberg and Craig Kolb and Tom{'a}{\v s} Davidovi{\v c} and Kai-Hwa Yao and Theresa Foley and Yong He and Lifan Wu and Lucy Chen and Tomas Akenine-M{\"o}ller and Chris Wyman and Cyril Crassin and Nir Benty},
   title =       {The {Falcor} Rendering Framework},
   year =        {2022},
   month =       {8},
   url =         {https://github.com/NVIDIAGameWorks/Falcor},
   note =        {\url{https://github.com/NVIDIAGameWorks/Falcor}}
}
```
