# Pathfinding Workshop

A Direct3D 11 interactive workshop application that teaches fundamental pathfinding and computational-geometry algorithms through hands-on exercises. Students fill in exercise stubs in `UserPathfindingWorkSheet.h`, and the application renders the results side-by-side with the reference solution from `ControlPathfindingWorkSheet.h`.

---

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Building](#building)
3. [Project Structure](#project-structure)
4. [Architecture Overview](#architecture-overview)
5. [Module Reference](#module-reference)
   - [Pathfinding](#pathfinding)
   - [Render](#render)
   - [Utils](#utils)
   - [World](#world)
6. [Exercises](#exercises)

---

## Prerequisites

| Requirement | Notes |
|---|---|
| Visual Studio 2010 or later | C++ project (`.vcxproj`) |
| DirectX SDK (June 2010) | Install and set `%DXSDK_DIR%` |
| Windows SDK | Provides `d3d11.lib`, `dxgi.lib`, `d3dcompiler.lib` |

The project links against `d3d11.lib`, `dxgi.lib`, `d3dcompiler.lib`, and the DirectX Math/Texture helpers from the DirectX SDK.  
No D3DX geometry or effect-framework dependency is required.

---

## Building

1. Open `Sources/Tutorial.sln` in Visual Studio.
2. Ensure `$(DXSDK_DIR)` resolves to your DirectX SDK installation directory.
3. Select the desired configuration (`Debug` / `Release`) and platform (`Win32` / `x64`).
4. Build the solution (`Ctrl+Shift+B`).
5. The executable is written to `bin/`.

---

## Project Structure

```
Sources/
└── Tutorial/
    ├── Tutorial.cpp              # Application entry point (WinMain, message loop, Update/Render)
    ├── pch.h / pch.cpp           # Precompiled header (Windows, D3D11, D3DX)
    ├── framework.h               # Minimal Windows include wrapper
    ├── targetver.h               # Target Windows version
    │
    ├── Pathfinding/              # Workshop exercise system
    │   ├── PathfindingWorkSheet.h            # Abstract base class for exercises
    │   ├── PathfindingWorkSheet.cpp          # (empty – stubs live in the header)
    │   ├── ControlPathfindingWorkSheet.h     # Reference (correct) solutions
    │   ├── UserPathfindingWorkSheet.h        # Student implementation stubs
    │   └── PathfindingWorkshopManager.h/.cpp # Singleton that drives both worksheets
    │
    ├── Render/                   # Rendering sub-system
    │   ├── RenderManager.h/.cpp  # D3D11 device, swap-chain, common states, CBs
    │   ├── TexturesManager.h/.cpp# Texture cache (load-once, keyed by filename)
    │   ├── ObjectsManager.h/.cpp # 3-D object cache (load-once, keyed by filename+scale)
    │   ├── EntitiesManager.h/.cpp# Scene entity factory and list
    │   ├── PostProcess.h/.cpp    # HDR tone-mapping post-process pass
    │   ├── Sky.h/.cpp            # Sky-dome renderer with light/fog parameters
    │   ├── Clouds.h/.cpp         # Temporal cloud renderer (ping-pong render targets)
    │   └── Terrain.h/.cpp        # Height-map terrain renderer with patch grid
    │
    ├── Utils/                    # Core utility classes
    │   ├── AutoList.h            # Intrusive doubly-linked list (CRTP)
    │   ├── DynVec.h              # Dynamic array (malloc/realloc, no STL)
    │   ├── RefCounted.h          # Reference-counted base class
    │   ├── Camera.h/.cpp         # First-person camera with frustum
    │   ├── Frustum.h             # View-frustum planes and visibility tests
    │   ├── FrameTime.h/.cpp      # High-resolution frame-delta timer
    │   ├── Input.h/.cpp          # Keyboard + XInput gamepad input
    │   ├── Entity.h/.cpp         # Scene entity (object + transform + bounding volumes)
    │   ├── Object.h/.cpp         # 3-D renderable object (meshes + materials)
    │   ├── Mesh.h/.cpp           # Vertex/index buffer wrapper
    │   ├── Material.h/.cpp       # Material (texture slot list)
    │   ├── Shader.h/.cpp         # Shader wrappers (VS, PS, GS, CS) with hot-reload
    │   ├── Texture.h/.cpp        # D3D11 texture wrapper (file, RT, DS, CPU-write)
    │   └── Utils.h/.cpp          # Assertion, string conversion, and common macros
    │
    ├── World/
    │   └── World.h/.cpp          # World loader (XML scene description)
    │
    └── Extern/                   # Third-party libraries (unmodified)
        ├── tiny_obj_loader.h/.cc # OBJ mesh loader
        └── tinyxml2.h/.cpp       # XML parser
```

---

## Architecture Overview

```
WinMain (Tutorial.cpp)
  │
  ├─ RenderManager  (D3D11 device, swap-chain, common GPU states)
  ├─ TexturesManager  (texture cache)
  ├─ ObjectsManager   (3-D object cache)
  ├─ EntitiesManager  (scene entity list)
  └─ PathfindingWorkshopManager
        ├─ UserPathfindingWorkSheet    ← student fills this in
        └─ ControlPathfindingWorkSheet ← reference solution
```

Each frame:

1. **Input** — `Input::Update()` polls XInput and keyboard state.
2. **Camera** — `Camera::UpdateBasedOnInput()` moves the first-person camera.
3. **Workshop** — `PathfindingWorkshopManager::Update(dt)` ticks both worksheets.
4. **Rendering**:
   - Clear HDR render target.
   - Render sky-dome (`Sky::Render`).
   - Render terrain (`Terrain::Render`).
   - Render world entities (`RenderManager::RenderEntities`).
   - Render workshop entities.
   - Render clouds (`Clouds::Render`).
   - Post-process tone-mapping (`PostProcess::Render`).
   - Present swap-chain.

---

## Module Reference

### Pathfinding

#### `PathfindingWorkSheet` (abstract base)

Defines the interface that both the control (reference) sheet and the user sheet must implement. It also provides:
- `Update(float dt)` — per-frame logic tick.
- `Render()` — per-frame draw.

Type aliases declared here map D3DX math types to shorter names (`Vector2`, `Vector3`, `Vector4`, `Mat4x4`, `Color`) that are used throughout the exercises.

#### `ControlPathfindingWorkSheet`

Contains the **correct reference implementations** of all exercises.  
Students can compare their output against these results at runtime.

#### `UserPathfindingWorkSheet`

Contains **stub implementations** that students must complete.  
Stubs return placeholder values (`false` / `0`) until implemented.

#### `PathfindingWorkshopManager` (singleton)

Owns both worksheet instances and the list of workshop-specific scene entities.  
Drives `Update` and `Render` on both sheets each frame.  
Access via the `g_pathfindingWorkshopManager` global macro.

---

### Render

#### `RenderManager` (singleton)

Central hub for all Direct3D 11 rendering.  Responsibilities:
- Device and swap-chain creation/destruction.
- Common GPU pipeline states (depth-stencil, rasterizer, blend, samplers).
- Constant-buffer management (`LockConstantBuffer` / `UnlockConstantBuffer`).
- Per-frame CB upload (`SetPerFrameConstantBuffers`).
- High-level draw calls: `RenderObject`, `RenderEntities`, `RenderQuadMesh`.
- Owns `Camera`, `PostProcess`, `Sky`, and `Clouds`.

Access via the `g_renderManager` global macro.

#### `TexturesManager` (singleton)

Texture cache keyed by filename. Calling `Load()` more than once for the same file returns the already-loaded `Texture*`.  
Access via `g_texturesManager`.

#### `ObjectsManager` (singleton)

3-D object cache keyed by filename **and** scale factor. Reuses loaded objects sharing both attributes.  
Access via `g_objectsManager`.

#### `EntitiesManager` (singleton)

Factory and owner of all `Entity` instances in the scene.  
Access via `g_entitiesManager`.

#### `PostProcess`

Reads an HDR colour buffer and a depth buffer and applies tone-mapping to produce the final LDR image.

#### `Sky`

Renders a sky-dome and exposes parameters for the directional light, ambient light, sky colour, and atmospheric fog, which are consumed by terrain and object shaders.

#### `Clouds`

Temporal cloud renderer that accumulates cloud coverage across frames using two ping-pong render targets (`m_cloudsRenderTarget[2]`).

#### `Terrain`

Height-map terrain divided into a patch grid for frustum-culled rendering.  
Provides `GetHeight(x, z)` for world-space height queries (bilinear interpolation) and `GetMinMaxHeights` for range queries over an AABB.

---

### Utils

#### `AutoList<T>` (template)

Intrusive doubly-linked list using the CRTP pattern.  
Any class that inherits `AutoList<MyClass>` is automatically inserted into the global list at construction and removed at destruction.  
Used by the `Shader` hierarchy to enable `Shader::ReloadAll()` and `Shader::UnloadAll()`.

#### `DynVec<T>` (template)

A resizable array backed by `malloc`/`realloc`.  Designed for plain-data types — no copy constructor is called on elements.  
Key methods: `Add`, `Insert`, `Remove` (swap-with-last), `RemoveKeepOrder`, `Find`, `SetAllocParams`.

#### `RefCounted`

Base class for COM-style manual reference counting.  
`AddRef()` increments, `Release()` decrements and `delete this` when the count reaches zero.  
Used by `Texture`, `Object`, `Material`, and `Entity`.

#### `Camera`

First-person perspective camera.  
- `SetViewParams` / `SetProjParams` — configure view and projection matrices.
- `UpdateBasedOnInput` — translate and rotate the camera from analogue axis values.
- `IsAABBInFrustum` / `IsBoundingSphereInFrustum` — inline frustum visibility tests.

#### `Frustum` (`sFrustum`)

Six-plane view frustum.  
`ComputeFrustum` rebuilds the planes from the camera world matrix and projection parameters.  
`BSvsFrustumTest` and `AABBvsFrustumTest` return `FRT_OUT`, `FRT_IN_OUT`, or `FRT_IN`.

#### `FrameTime`

High-resolution frame timer using `QueryPerformanceCounter`.  
Call `Init()` once at startup and `Update()` each frame.  
`GetDt()` returns seconds since the last frame; `GetTime()` returns total elapsed seconds.

#### `Input`

Manages keyboard state and up to `XUSER_MAX_COUNT` XInput gamepad states.  
Call `ProcessMessage` from `WndProc` to capture keyboard events.  
Call `Update()` each frame to poll gamepads.

#### `Entity`

A scene object: holds a pointer to a shared `Object`, a world-space transform matrix, and pre-computed bounding sphere / AABB (derived from the object's local bounds transformed by the world matrix).  Inherits `RefCounted`.

#### `Object`

A renderable 3-D asset loaded from an OBJ file. Owns lists of `Mesh` and `Material` objects, plus local-space bounding volumes.  Inherits `RefCounted`.

#### `Mesh`

Thin wrapper around a D3D11 vertex buffer and index buffer.  
`Draw()` binds the buffers and issues the indexed draw call.

#### `Material`

Holds a list of `Texture*` slots (diffuse, normal, roughness, specular, AO).  
Inherits `RefCounted`.

#### `Shader` / `ShaderVS` / `ShaderPS` / `ShaderGS` / `ShaderCS`

Shader wrappers that compile HLSL from file.  
Base class `Shader` inherits `AutoList<Shader>` so that `Shader::ReloadAll()` can hot-reload every shader without explicit bookkeeping.  
`bIsFileChanged()` detects on-disk modifications for live reloading.

#### `Texture`

D3D11 texture wrapper supporting four usage modes (`eTextureType_Default`, `eTextureType_CPUWrite`, `eTextureType_RenderTarget`, `eTextureType_DepthStencil`).  
Provides `Lock`/`Unlock` for CPU-write access, `VSSet`/`PSSet`/`CSSet` to bind shader-resource views, and `GetRenderTargetView`/`GetDepthStencilView` accessors per mip level.  Inherits `RefCounted`.

#### `Utils.h`

Common macros used throughout the codebase:

| Macro | Purpose |
|---|---|
| `myAssert(cond, msg)` | Shows an error message box if `cond` is false |
| `V_RETURN(x)` | Evaluates an `HRESULT` expression and returns on failure |
| `SAFE_DELETE(x)` | Null-checks, deletes, and nulls a pointer |
| `SAFE_RELEASE(x)` | Null-checks, calls `Release()`, and nulls a pointer |
| `SAFE_FREE(x)` | Null-checks, calls `free()`, and nulls a pointer |
| `D3D_DEBUG_SET_NAME(x, name)` | Attaches a debug name to a D3D11 object |

---

### World

#### `World`

Loads an XML scene description (via `tinyxml2`) that references object files and their placement transforms.  Populates an `Entity` list and creates the `Terrain`.

---

## Exercises

Exercises are defined as pure virtual methods on `PathfindingWorkSheet`.  
Students implement them in `Sources/Tutorial/Pathfinding/UserPathfindingWorkSheet.h`.

### Exercise 1 — Line Side Test

**`float SignedArea(Vector2 p1, Vector2 p2, Vector2 p3)`**

Returns twice the signed area of the triangle formed by the three 2-D points.  The sign indicates the winding order:
- **Positive** → counter-clockwise (p3 is to the left of the directed edge p1→p2).
- **Negative** → clockwise (p3 is to the right).
- **Zero** → the three points are collinear.

Formula: `p1.x*(p2.y - p3.y) + p2.x*(p3.y - p1.y) + p3.x*(p1.y - p2.y)`  
(Equivalent cross-product expansion.)

**`bool IsLeft(Vector2 p1, Vector2 p2, Vector2 p3)`**

Returns `true` when point `p3` lies strictly to the **left** of the directed line from `p1` to `p2`.  
Implemented by checking `SignedArea(p1, p2, p3) > FLT_EPSILON`.
