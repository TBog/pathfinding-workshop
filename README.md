# pathfinding-workshop

An interactive 2-D pathfinding visualiser built with **Direct3D 11** from the
**Microsoft DirectX SDK (June 2010)**.

## Requirements

| Requirement | Version |
|---|---|
| Visual Studio | 2010 (toolset `v100`) |
| DirectX SDK | [June 2010](https://www.microsoft.com/en-us/download/details.aspx?id=6812) |
| OS | Windows Vista / 7 / 8 / 10 (or later) |

The DirectX SDK installer sets the `DXSDK_DIR` environment variable.  The
project files rely on that variable for include and library paths, so no manual
path configuration is required.

## Building

1. Open `pathfinding-workshop.sln` in Visual Studio 2010.
2. Select a configuration (`Debug` or `Release`) and platform (`Win32` or
   `x64`).
3. Press **F7** (or **Build → Build Solution**).

The resulting executable is placed in `Debug\` or `Release\` under the solution
root.

## Running

Launch `pathfinding-workshop.exe`.  A window opens showing a 20 × 15 grid of
cells (each 40 × 40 pixels).

### Controls

| Key / Mouse | Action |
|---|---|
| **1** | Switch to *Wall* mode |
| **2** | Switch to *Start* mode (places the green start cell) |
| **3** | Switch to *End* mode (places the red end cell) |
| **4** | Switch to *Erase* mode |
| **C** | Clear the entire grid |
| **Esc** | Quit |
| **Left-click / drag** | Apply current mode to the cell under the cursor |
| **Right-click / drag** | Erase the cell under the cursor |

### Cell colours

| Colour | Meaning |
|---|---|
| Light grey | Empty / passable |
| Dark grey | Wall / impassable |
| Green | Start position |
| Red | End position |
| Light blue | Visited by the search algorithm |
| Yellow | Final path |

## Project structure

```
pathfinding-workshop/
├── pathfinding-workshop.sln              Visual Studio 2010 solution
├── pathfinding-workshop.vcxproj          Visual Studio 2010 project
├── pathfinding-workshop.vcxproj.filters  Source-file organisation
└── src/
    ├── main.cpp      Win32 entry point and message loop
    ├── App.h/.cpp    Application class – grid state and input handling
    └── Renderer.h/.cpp  Direct3D 11 renderer – device, shaders, draw calls
```

## Architecture notes

* **Renderer** initialises a Direct3D 11 device and DXGI swap chain, compiles
  inline HLSL shaders at runtime with `D3DCompile` (from `d3dcompiler.h`), and
  draws each grid cell as a coloured quad using an indexed unit-quad mesh.
* **App** owns the grid state and maps mouse/keyboard input to cell-type
  changes.  It is intentionally decoupled from the renderer so that pathfinding
  algorithm steps can be added in `App::Update()` later.
* The orthographic projection matrix maps screen-pixel coordinates directly to
  NDC without any additional scaling; cells are positioned by updating a
  per-draw-call constant buffer (`PerDrawCB`) before each `DrawIndexed` call.
