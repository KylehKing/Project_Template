# Visual Studio 2022 Project Overview

I have used Visual Studio 2022 to create my project on Windows 11.

## Code Structure

The code is organized as follows:

- **scenebasic_uniform.h**: Defines variables to be used throughout the program.

- **scenebasic_uniform.cpp**: Contains the C++ logic for the scene, including:
  - Initializing the scene
  - Rendering the scene
  - Updating parts of the scene every frame
  - Setting uniform variables that get passed to the fragment and vertex shaders
  - A two-pass rendering approach: Pass 1 renders the scene to a texture (using an FBO), and Pass 2 applies post-processing effects.

- **Fragment Shader**: Handles shader logic that requires accuracy, while the vertex shader is for less accurate but faster shading techniques.

## Implementation Details

**scenebasic_uniform.cpp** defines uniform variables for the shaders such as:
- Fog values
- Textures
- Normal map
- Materials
- Lights
- Matrices (defined in `setMatrices`)
- Edge detection threshold

The **fragment shader** processes these inputs and performs:
- **In Pass 1:**
  - TBN matrix calculations for normal maps
  - Fog calculations for simulating atmospheric effects
  - BlinnPhong function for light rendering
- **In Pass 2:**
  - Sampling from the texture rendered in Pass 1
  - Edge detection calculations using the sampled texture and a threshold uniform

**scenerunner.h** captures keyboard and mouse inputs through GLFW callbacks, which update variables in scenebasic_uniform.cpp to control the camera.

## Demo Video

[Watch the demo video](https://youtu.be/cHe1jB72Hm8)
[GitHub Link](https://github.com/KylehKing/Project_Template/tree/CW2)
