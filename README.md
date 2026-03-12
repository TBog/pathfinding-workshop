# Pathfinding Workshop

An interactive, hands-on educational framework for learning **computational geometry** and **pathfinding algorithms** through real-time 3D visualization, built with **Direct3D 11** and **C++**.

Students implement geometric primitives (signed area, line-side test, and more) in a dedicated worksheet class, then immediately visualize and verify their solutions against a reference implementation — all within a fully featured 3D rendered environment.

---

## Table of Contents

- [Key Features](#key-features)
- [Folder Structure](#folder-structure)
- [Dependencies & Requirements](#dependencies--requirements)
- [Installation & Build](#installation--build)
- [Usage](#usage)
- [Pathfinding Exercises](#pathfinding-exercises)
- [Architecture Overview](#architecture-overview)
- [Contributing](#contributing)
- [License](#license)

---

## Key Features

- **Exercise Framework** — A base class (`PathfindingWorkSheet`) defines the interface for every exercise. Students fill in `UserPathfindingWorkSheet`; the built-in `ControlPathfindingWorkSheet` provides the reference solution for comparison.
- **Real-Time 3D Visualization** — Direct3D 11 renderer with frustum culling, HDR render targets, sky, terrain, and volumetric clouds.
- **Shader Hot-Reload** — Press **Alt + R** at runtime to recompile all HLSL shaders without restarting the application.
- **Gamepad & Keyboard Support** — XInput gamepad (Xbox controller) and keyboard camera controls out of the box.
- **XML-Driven Scenes** — World configuration, terrain, sky, clouds, and post-processing settings are all declared in XML files under `bin/Worlds/`.
- **OBJ Model Loading** — 3D models are loaded via the bundled `tiny_obj_loader` library.
- **Template Utilities** — Custom `DynVec<T>` dynamic array, `RefCounted` reference counting, and `AutoList<T>` linked-list helpers.

---

## Folder Structure

```
pathfinding-workshop/
├── README.md
├── Sources/
│   ├── Tutorial.sln                 # Visual Studio 2022 solution
│   └── Tutorial/                    # Main C++ project
│       ├── Tutorial.cpp             # Application entry point & game loop
│       ├── pch.h / pch.cpp          # Precompiled headers
│       ├── framework.h              # Windows/DirectX includes
│       ├── Pathfinding/             # Workshop exercise classes
│       │   ├── PathfindingWorkSheet.h           # Abstract base class (interface)
│       │   ├── PathfindingWorkSheet.cpp         # Shared update/render stub
│       │   ├── UserPathfindingWorkSheet.h       # ← Student implementation goes here
│       │   ├── ControlPathfindingWorkSheet.h    # Reference solution
│       │   ├── PathfindingWorkshopManager.h     # Singleton manager
│       │   └── PathfindingWorkshopManager.cpp
│       ├── Render/                  # Direct3D 11 rendering subsystem
│       │   ├── RenderManager.h/cpp  # D3D11 device, swap chain, constant buffers
│       │   ├── Camera.h/cpp         # View/projection, frustum culling
│       │   ├── EntitiesManager.h/cpp
│       │   ├── ObjectsManager.h/cpp # OBJ model loading & caching
│       │   ├── TexturesManager.h/cpp
│       │   ├── Sky.h/cpp
│       │   ├── Clouds.h/cpp         # Volumetric cloud rendering
│       │   ├── Terrain.h/cpp        # Heightmap terrain
│       │   └── PostProcess.h/cpp    # HDR tone mapping
│       ├── Utils/                   # Shared utilities & data structures
│       │   ├── DynVec.h             # Dynamic array template
│       │   ├── RefCounted.h         # Reference-counted base class
│       │   ├── AutoList.h           # Intrusive linked list
│       │   ├── Entity.h/cpp         # Scene entity (transform + mesh)
│       │   ├── Camera.h/cpp
│       │   ├── Input.h/cpp          # XInput + keyboard polling
│       │   ├── Shader.h/cpp         # HLSL compilation & binding
│       │   ├── Texture.h/cpp        # Texture2D, render targets, depth buffers
│       │   ├── Mesh.h/cpp           # Vertex/index buffer wrapper
│       │   ├── Object.h/cpp         # 3D object (mesh + material)
│       │   ├── Frustum.h/cpp        # Camera frustum planes
│       │   ├── FrameTime.h/cpp      # Delta-time & FPS counter
│       │   └── Utils.h/cpp          # Assertion macros & helpers
│       ├── World/
│       │   └── World.h/cpp          # XML scene loader
│       └── Extern/                  # Vendored third-party libraries
│           ├── tinyxml2.h/cpp       # XML parsing
│           └── tiny_obj_loader.h/cc # OBJ model loading
└── bin/                             # Runtime assets & output binary
    ├── Shaders/                     # HLSL source files (.fx / .h)
    ├── Textures/                    # Material textures
    ├── Models/                      # OBJ 3D models
    └── Worlds/                      # XML scene configuration files
        ├── world_pathfind.xml       # Default pathfinding scene
        ├── world1.xml, world2.xml
        ├── terrain.xml
        ├── sky.xml
        ├── clouds.xml
        └── postProcess.xml
```

---

## Dependencies & Requirements

### System Requirements

| Requirement | Minimum |
|---|---|
| OS | Windows 10 (64-bit) |
| GPU | DirectX 11–capable graphics card |
| IDE | Visual Studio 2022 |
| SDK | Windows 10 SDK (10.0) |
| DirectX | DirectX SDK (June 2010) or equivalent D3DX headers |

### Libraries Used

| Library | Purpose | Included |
|---|---|---|
| **Direct3D 11** (`d3d11.lib`) | GPU rendering API | Via Windows SDK |
| **D3DX11** (`d3dx11.lib`) | Math & texture utilities | Via DirectX SDK |
| **DXGI** (`dxgi.lib`) | Swap chain & display | Via Windows SDK |
| **XInput** (`xinput.lib`) | Xbox controller input | Via Windows SDK |
| **tinyxml2** | XML scene file parsing | Vendored in `Extern/` |
| **tiny_obj_loader** | OBJ 3D model loading | Vendored in `Extern/` |

> **Note:** The project references the DirectX SDK via the `$(DXSDK_DIR)` environment variable. Ensure the DirectX SDK (June 2010) is installed, or update the include/library paths in `Tutorial.vcxproj` to point to a compatible DXSDK location.

---

## Installation & Build

### 1. Install Prerequisites

- **Visual Studio 2022** with the *Desktop development with C++* workload
- **DirectX SDK (June 2010)** — [Download from Microsoft](https://www.microsoft.com/en-us/download/details.aspx?id=6812)
  - The installer sets the `DXSDK_DIR` environment variable automatically.
  - If you use a custom location, update the include and library directories in `Tutorial.vcxproj`.

### 2. Open the Solution

```
Sources/Tutorial.sln
```

Open this file in Visual Studio 2022.

### 3. Build

Select a configuration (**Debug** or **Release**) and platform (**Win32**), then build:

- **Menu:** *Build → Build Solution*
- **Keyboard:** `Ctrl + Shift + B`

The compiled binary and assets are output to the `bin/` directory.

### 4. Run

Set `Tutorial` as the startup project and press **F5** (or run the `.exe` from `bin/`). The application requires the `Shaders/`, `Textures/`, `Models/`, and `Worlds/` folders to be present in the same directory as the executable — the `bin/` folder contains all of these.

---

## Usage

### Camera Controls

| Action | Gamepad (Xbox) | Keyboard |
|---|---|---|
| Move forward/back | Left stick Y | — |
| Strafe left/right | Left stick X | — |
| Look / rotate | Right stick | — |
| 10× speed boost | Right trigger (> 25%) | — |
| 100× speed boost | Right shoulder button | — |

### Runtime Shortcuts

| Shortcut | Action |
|---|---|
| **Alt + R** | Hot-reload all HLSL shaders |

### Switching the Active World

Edit `Tutorial.cpp` to change the XML world file loaded at startup, or modify any of the XML files under `bin/Worlds/` to adjust scene content, terrain, sky, clouds, or post-processing settings.

---

## Pathfinding Exercises

The workshop is structured around a **worksheet pattern**:

| Class | Role |
|---|---|
| `PathfindingWorkSheet` | Abstract base — defines the exercise interface |
| `UserPathfindingWorkSheet` | **Your implementation** — fill in each exercise here |
| `ControlPathfindingWorkSheet` | Reference solution — used for comparison/verification |

### Exercise 1 — Line Side Test

#### `SignedArea(p1, p2, p3) → float`

Computes the **signed area** of the triangle formed by three 2D points using the cross product of edge vectors:

```
SignedArea = p1.x·p2.y + p2.x·p3.y + p3.x·p1.y
           − p1.x·p3.y − p3.x·p2.y − p2.x·p1.y
```

| Return value | Meaning |
|---|---|
| `> 0` | Points are in **counter-clockwise** order; `p3` is **left** of line `p1→p2` |
| `< 0` | Points are in **clockwise** order; `p3` is **right** of line `p1→p2` |
| `≈ 0` | Points are **collinear** |

#### `IsLeft(p1, p2, p3) → bool`

Returns `true` when `p3` lies strictly to the **left** of the directed line from `p1` to `p2`:

```cpp
return SignedArea(p1, p2, p3) > FLT_EPSILON;
```

> **Note:** The reference solution uses `FLT_EPSILON` as the comparison threshold. `FLT_EPSILON` is the smallest difference between `1.0f` and the next representable `float`, which works for unit-scale coordinates but may need to be adjusted to a larger absolute tolerance when your geometry uses very large or very small coordinate values.

#### Why these operations matter for pathfinding

These two primitives are the building blocks for:

- **Point-in-polygon testing** — determine whether an agent is inside a walkable region
- **Line-segment intersection** — detect when a path crosses an obstacle
- **Convex hull construction** — build collision or navigation boundaries (e.g., Graham scan)
- **Navigation mesh (NavMesh) validation** — verify winding order and mesh integrity
- **Path smoothing** — check straight-line visibility between two waypoints

### Implementing Your Worksheet

Open `Sources/Tutorial/Pathfinding/UserPathfindingWorkSheet.h` and replace the placeholder `return false;` stubs with your own implementations:

```cpp
// UserPathfindingWorkSheet.h
float SignedArea(Vector2 p1, Vector2 p2, Vector2 p3) override
{
    // TODO: implement
    return 0.f;
}

bool IsLeft(Vector2 p1, Vector2 p2, Vector2 p3) override
{
    // TODO: implement
    return false;
}
```

The `PathfindingWorkshopManager` will call both your implementation and the reference solution each frame so you can compare results in real time.

---

## Architecture Overview

### Design Patterns

**Singleton Managers** — Each subsystem exposes a static `Get()` accessor and a corresponding global macro:

```cpp
#define g_renderManager             RenderManager::Get()
#define g_pathfindingWorkshopManager PathfindingWorkshopManager::Get()
```

Managers are created in `InitializeApp()` and destroyed in `DestroyApp()`.

**Reference Counting** — `RefCounted` is the base class for all shared resources (textures, meshes, materials, objects). `AddRef()` / `Release()` handle lifetime; `Release()` auto-deletes when the count reaches zero.

**`DynVec<T>`** — A custom dynamic array used throughout the codebase in place of `std::vector`, with `Add`, `Remove`, `RemoveKeepOrder`, `Find`, and bounds-checked `operator[]`.

### Rendering Pipeline (per frame)

```
1. SetPerFrameConstantBuffers     — upload camera matrices, time, etc.
2. ClearBackBufferRenderTarget
3. Clouds::PreRender              — render cloud shadow/density map
4. SetRenderTargets(HDR, Depth)   — switch to HDR off-screen buffer
5. Sky::Render                    — render sky dome
6. ComputeVisibleEntities         — frustum cull (sphere + AABB tests)
7. RenderManager::RenderEntities  — draw visible scene objects
8. Terrain::Render                — draw heightmap terrain
9. Clouds::Render                 — composite clouds over scene
10. PostProcess::Render           — tone-map HDR → LDR, write to back buffer
11. Present                       — swap chain flip
```

### HLSL Shaders

All shaders live in `bin/Shaders/` as `.fx` files (Shader Model 5.0). They are compiled at build time to `.cso` files in `Temp/$(Configuration)/Shaders/`. Hot-reload at runtime is triggered with **Alt + R**.

| Shader set | Files |
|---|---|
| Objects | `Object_VS.fx`, `Object_PS.fx` |
| Terrain | `Terrain_VS.fx`, `Terrain_PS.fx` |
| Sky | `Sky_VS.fx`, `Sky_PS.fx` |
| Clouds | `Clouds_VS.fx`, `Clouds_PS.fx`, `Clouds_base_PS.fx`, `Clouds_details_PS.fx`, `Clouds_apply_PS.fx` |
| Post-processing | `PostProcess_VS.fx`, `PostProcess_PS.fx`, `FullScreenQuad_VS.fx`, `FullScreenQuad_PS.fx` |
| Shared headers | `Common_VS.h`, `Common_PS.h`, `Lighting.h`, `Utils.h`, `Clouds_utils.h` |

---

## Contributing

Contributions are welcome! Please follow these guidelines:

1. **Fork** the repository and create a feature branch from `main`.
2. Keep changes focused — one feature or fix per pull request.
3. Follow the existing code style (indentation with tabs, `PascalCase` for classes and methods, `m_` prefix for member variables, `g_` prefix for globals).
4. When adding a new exercise, follow the worksheet pattern:
   - Add the pure-virtual method to `PathfindingWorkSheet.h`.
   - Add the reference implementation to `ControlPathfindingWorkSheet.h`.
   - Leave a stub (`return false;` / `return 0.f;`) in `UserPathfindingWorkSheet.h`.
5. Test on both **Debug** and **Release** configurations before opening a pull request.
6. Update this README if you add new exercises, controls, or build requirements.

---

## License

This project does not currently include an explicit license file. Please contact the repository owner for licensing information before using this code in your own projects.