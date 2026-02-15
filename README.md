# Thomas Attractor & Chaos Systems Simulation

This repository contains C and C++ implementations of various Strange Attractors (Thomas, Lorenz, Aizawa), rendered both in the terminal (using ASCII/ANSI) and in a standalone window using OpenGL.
# Made By Re

## 📁 Project Structure

*   **`attractor.cpp`**: (C++) Advanced terminal-based rendering engine supporting multiple chaos systems (Thomas, Lorenz, Aizawa) with 3D projection, rotation, and TrueColor "glow" effects.
*   **`thomasgl.cpp`**: (C++) High-performance OpenGL implementation. Creates a dedicated window (`WS_POPUP`) to render the Thomas Attractor with hardware acceleration, vertex blending, and smooth rotations.
*   **`attractor.c`**: (C) A pure C implementation of the terminal renderer, focusing on Thomas and Lorenz attractors.
*   **`main.cpp`**: Entry point or auxiliary test file for the project.

---

## 🛠 Prerequisites

To run and compile these files, you need a C++ compiler and the Windows SDK libraries (usually pre-installed or included with the compiler).

### 1. C++ Compiler (MinGW-w64)
You need `g++` (GCC) to compile the code. We recommend MinGW-w64 for Windows.
*   **Download**: [MinGW-w64 via MSYS2](https://www.msys2.org/) or [w64devkit](https://github.com/skeeto/w64devkit/releases)
*   *Installation*: Download the installer/zip, extract it, and add the `bin` folder to your system PATH.

### 2. OpenGL Libraries
*   **Status**: **Pre-installed** on Windows.
*   The required libraries (`opengl32.lib`, `gdi32.lib`, `user32.lib`) come standard with Windows. You do **not** need to download them separately if you have a standard build environment.

---

##  How to Compile & Run

Open your terminal (PowerShell or CMD) in this directory and use the following commands.

### 1. Compile the OpenGL Version (`thomasgl.cpp`)
This version runs in a high-performance graphical window.
```powershell
g++ thomasgl.cpp -o thomasgl -lopengl32 -lgdi32 -luser32
```
*   **Run**: `./thomasgl`
*   **Controls**:
    *   `ESC`: Close the window.
    *   `Right Click`: Close the window.
    *   `Left Click + Drag`: Move the window (since it is borderless).

### 2. Compile the C++ Terminal Version (`attractor.cpp`)
This version runs directly inside your command prompt using text characters.
```powershell
g++ attractor.cpp -o attractor
```
*   **Run**: `./attractor`
*   **Note**: For best results, use a terminal that supports TrueColor (like **Windows Terminal** or VS Code Integrated Terminal) and decrease your font size slightly.

### 3. Compile the C Version (`attractor.c`)
```powershell
gcc attractor.c -o c_attractor
```
*   **Run**: `./c_attractor`

---

##  Dependencies & Links

| Component | Usage | Download Link |
| :--- | :--- | :--- |
| **MinGW-w64** | C/C++ Compiler | [Download Here](https://github.com/skeeto/w64devkit/releases) |
| **Windows Terminal** | Best for viewing terminal output | [Microsoft Store](https://apps.microsoft.com/detail/9n0dx20hk701) |
| **OpenGL** | Graphics Library | *(Included in Windows)* |

##  Controls Summary
*   **Terminal Versions**: Runs automatically. `Ctrl+C` to stop.
*   **OpenGL Version**: 
    *   **Exit**: `ESC` key or Right-Click Mouse.
    *   **Move**: Drag with Left Mouse Button.
