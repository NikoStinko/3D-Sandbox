# 3D-Sandbox

A simple 3D sandbox using modern OpenGL and GLFW.

## Prerequisites

- G++
- CMake
- GLFW
- GLEW
- Assimp
- GLM

## Installation

### Linux

#### Ubuntu/Debian

```bash
sudo apt update
sudo apt install cmake libglfw3-dev libglew-dev libassimp-dev libglm-dev
```

#### Fedora

```bash
sudo dnf install cmake glfw-devel glew-devel assimp-devel glm-devel
```

#### Arch Linux

```bash
sudo pacman -S cmake glfw glew assimp glm
```

### MacOS

```bash
brew update
brew install cmake glfw glew assimp glm
```

### Windows

#### Install vcpkg
```bash
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

#### Integrate vcpkg with CMake

```bash
vcpkg integrate install
```

#### Install Dependencies
```bash
vcpkg install glfw3 glew assimp glm
```

## Compile and Run

```bash
mkdir build
cd build
cmake ..
make
./3D-Sandbox
```
