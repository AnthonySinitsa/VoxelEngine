# Vulkan Rendering Engine
![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg) ![Vulkan](https://img.shields.io/badge/Vulkan-1.3+-red.svg) ![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows-lightgrey.svg)

A high-performance C++20 rendering engine designed for real-time particle simulations and galactic kinematics. This project focuses on high-throughput rendering, capable of handling millions of particles with low frame latency.

![2mil](images/2milStars.png)
*2 million stars rendered with an average 7ms frame time.*

---

## Overview
This engine was built to explore the limits of Vulkan's compute and graphics pipelines. The flagship simulation implements a spiral galaxy model based on density wave theory and galactic coordinate math.

### Key Features
- **Massive Particle Counts:** Optimized vertex and storage buffers for simulating up to 2 million active stars.
- **Scientific Accuracy:** Implements de Vaucouleurs's Law and spiral density wave math for realistic galactic distributions.
- **Modern C++ Pipeline:** Full C++20 implementation with automated dependency management via CMake FetchContent.
- **Cross-Platform:** Developed for Windows and Linux (Optimized for Arch-based distros).
- **Integrated Tooling:** UI management via Dear ImGui (Docking branch) for real-time parameter tuning.

---

## Visuals

| Galactic Motion | Scene Management |
| :--- | :--- |
| ![1.83](images/1.83radius.gif) | ![scene](images/Scenemanagement.gif) |
| *Simulation of orbital velocity and radius* | *Dynamic object and scene hierarchy* |

---

## Technical Stack
- **API:** Vulkan 1.3
- **Windowing:** GLFW
- **Math:** GLM (OpenGL Mathematics)
- **UI:** Dear ImGui
- **Shaders:** GLSL (compiled to SPIR-V)

### Project Architecture
Detailed breakdowns of the engine modules can be found in the documentation:
- 🪟 [**Windowing System**](README/Windowing.md) - Surface and swapchain management.
- 🎨 [**Graphics Pipeline**](README/GraphicsPipeline.md) - Descriptor sets and pipeline layouts.
- 🖌️ [**Drawing Process**](README/Drawing.md) - Command buffer recording and synchronization.

---

## Getting Started

### Prerequisites
You must have the **Vulkan SDK** and a **C++20 compatible compiler** installed.

**Arch:**
```bash
sudo pacman -S base-devel cmake vulkan-devel
```

**Ubuntu / Debian:**
```bash
sudo apt install build-essential cmake libvulkan-dev vulkan-tools
```

### Build & Run
The engine uses an automated build script to handle CMake configuration and shader compilation.

```bash
chmod +x unixBuild.sh
./unixBuild.sh
```

---

## 🌐 Related Projects
I have also created a core reimplementation of this engine using **WebGPU** for browser accessibility (Currenty only works on Chromium browsers and only on windows as at the time of creation WebGPU didn't have linux implementation).

- **Live Demo:** [WebGPU Galaxy Simulation](https://webgpu-engine.netlify.app/)
- **Repository:** [WebGPU-Engine Source](https://github.com/AnthonySinitsa/webgpu-engine)

---

## References & Resources
- **Mathematics:** [Spiral Galaxy Renderer Theory](https://beltoforion.de/en/spiral_galaxy_renderer/) by Ingo Berg.
- **Formulas:** [Ellipse Geometry](https://www.desmos.com/calculator/qmu0f229zh) and [de Vaucouleurs's Law](https://www.desmos.com/calculator/bax4od28zj) Desmos visualizations.
- **Project Planning:** [Trello Board](https://trello.com/b/2YI795DN/voxelengine)

---
