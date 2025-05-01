# OpenGL 3D Car

This project helps you understand the basics of OpenGL by creating a 3D car object in a custom scene. It demonstrates the use of shaders, textures, lighting, camera controls, and basic 3D object rendering.

## Features

- Rendering 2D and 3D objects
- Camera movement using keyboard and mouse inputs
- Textures using the image format jpeg
- Simple lighting
- Shader program (vertex and fragment shaders)

## Prerequisites

Before you begin, ensure you have the following installed on your machine:
- C++ compiler
- OpenGL (version 3.3 or higher)
- GLFW (for window management and input handling)
- GLAD (for loading OpenGL functions)
- GLM (for matrix and vector operations)

## Installation

***Step 1:*** Clone the Repository
```bash
git clone https://github.com/edaerer/computer-graphics.git
cd computer-graphics
```

***Step 2:*** Install Dependencies
On Ubuntu:
```bash
sudo apt-get install libglfw3 libglfw3-dev libglm-dev
```
> [!NOTE]
> For GLAD: Add where/how to get it (since it's not apt-get installable).

***Step 3:*** Run the Project
```bash
make run
```

## Usage

Once the project is running, you can interact with the scene using the following controls:
- W/A/S/D: Move the camera forward, left, backward, and right.
- Mouse movement: Rotate the camera view.