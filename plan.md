# Source67 Engine Development Roadmap

Source67 is a high-performance 3D game engine inspired by Valve's Source and Source 2, built with C++ and OpenGL.

## Phase 1: Foundation (Core Systems)
- [ ] **Build System & Project Setup**
    - [ ] Configure CMake for cross-platform (macOS/Windows) support.
    - [ ] Integrate core dependencies: `GLFW` (Windowing), `GLAD` (OpenGL Loading), `GLM` (Math), `spdlog` (Logging).
- [ ] **Engine Core**
    - [ ] Implement a robust `Logger` and `Assertion` system.
    - [ ] Create a high-resolution `Timer` for delta time and frame tracking.
    - [ ] Standardize the File System (VFS) for asset path management.
- [ ] **Input Management**
    - [ ] Abstract Keyboard, Mouse, and Controller input.
    - [ ] Implement an Event System for decoupled communication between modules.

## Phase 2: Visuals (Rendering Engine)
- [ ] **OpenGL Renderer Base**
    - [ ] Shader abstraction (Compile/Link/Hot-reload).
    - [ ] Vertex Buffer, Index Buffer, and Vertex Array abstractions.
    - [ ] Texture management (Loading, Unit mapping, Mipmaps).
- [ ] **Advanced Rendering**
    - [ ] **PBR (Physically Based Rendering)**: Implement Metallic-Roughness workflow.
    - [ ] **Lighting**: Support for Directional, Point, and Spot lights.
    - [ ] **Shadow Mapping**: Implement cascaded shadow maps for large environments.
    - [ ] **Post-Processing**: Bloom, HDR, Tone Mapping, and FXAA/SMAA.
- [ ] **Scene Management**
    - [ ] Implement a Scene Graph or Entity Component System (ECS) using `EnTT`.
    - [ ] Frustum Culling for performance optimization.

## Phase 3: The Portal Problem (Non-Euclidean Mechanics)
- [ ] **Portal Rendering**
    - [ ] Implement Stencil Buffer slicing for portal views.
    - [ ] Recursive rendering support (Portals within portals).
    - [ ] Dynamic Clip Planes to prevent geometry clipping through portals.
- [ ] **Seamless Physical Transition**
    - [ ] Handle camera teleportation without frame jitter.
    - [ ] Recursive collision testing (predicting collisions through a portal).

## Phase 4: Physics & Dynamics
- [ ] **Physics Integration**
    - [ ] Integrate `Bullet Physics` or `Jolt Physics`.
    - [ ] Implement Rigid Body dynamics and complex collision shapes.
- [ ] **Portal Momentum Mechanics**
    - [ ] Transform velocity vectors correctly across portal planes ("Speedy thing goes in, speedy thing comes out").
    - [ ] Interpolate physics states across portals to prevent tunneling.

## Phase 5: Developer Experience (Tools & UI)
- [ ] **Engine UI (Editor)**
    - [ ] Integrate `ImGui` for real-time debugging and property editing.
    - [ ] Scene hierarchy viewer and asset browser.
- [ ] **Asset Pipeline**
    - [ ] Model loader (supporting modern formats like `glTF` or `FBX`).
    - [ ] Material system compatible with Source-like `.vmt` or modern JSON/YAML descriptions.
- [ ] **Hot Reloading**
    - [ ] Live shader editing and texture swapping.
    - [ ] (Optional) Dynamic library loading for game logic hot-reloading.

## Phase 6: Game Logic & Recreations
- [ ] Implement Player Controller (Source-like movement: air strafing, friction).
- [ ] Modular Weapon/Tool system (The Portal Gun).
- [ ] Level loading from standard formats.
