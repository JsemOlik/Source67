# Source67 Engine Development Roadmap

Source67 is a high-performance 3D game engine inspired by Valve's Source and Source 2, built with C++ and OpenGL.

## Phase 1: Foundation (Core Systems) [DONE]
- [x] **Build System & Project Setup**
    - [x] Configure CMake for cross-platform (macOS/Windows) support.
    - [x] Integrate core dependencies: `GLFW` (Windowing), `GLAD` (OpenGL Loading), `GLM` (Math), `spdlog` (Logging).
- [x] **Engine Core**
    - [x] Implement a robust `Logger` and `Assertion` system.
    - [x] Create a high-resolution `Timer` for delta time and frame tracking.
    - [x] Standardize the File System (VFS) for asset path management (VFS partially implemented via CMake/paths).
- [x] **Input Management**
    - [x] Abstract Keyboard, Mouse, and Controller input.
    - [x] Implement an Event System for decoupled communication between modules.

## Phase 2: Visuals (Rendering Engine) [/]
- [x] **OpenGL Renderer Base** [DONE]
    - [x] Shader abstraction (Compile/Link/Hot-reload support started)
    - [x] Vertex Buffer, Index Buffer, and Vertex Array abstractions
    - [x] Texture management (Loading, Unit mapping, Mipmaps)
- [ ] **Advanced Rendering** [/]
    - [x] **3D Foundation**: Integrated `PerspectiveCamera` and View-Projection matrices.
    - [x] **Basic 3D Rendering**: Verified with textured spinning cubes.
    - [x] **Lighting**: Implemented Blinn-Phong with Directional Light support.
    - [x] **Navigation**: Added FPS-style WASD movement and Mouse-look.
    - [ ] **PBR (Physically Based Rendering)**: Implement Metallic-Roughness workflow.
- [x] **Scene Management** [DONE]
    - [x] Implement a Basic Scene/Entity structure.

## Phase 3: The Portal Problem (Non-Euclidean Mechanics) [NEXT]
- [ ] **Portal Rendering**
    - [ ] Implement Stencil Buffer slicing for portal views.
    - [ ] Recursive rendering support.
- [ ] **Seamless Physical Transition**
    - [ ] Handle camera teleportation and collision predictions.

## Phase 4: Physics & Dynamics [DONE]
- [x] **Physics Integration** [DONE]
    - [x] Integrated `Jolt Physics`.
    - [x] Implement Rigid Body dynamics and basic shapes (Box).
    - [x] Transformation Sync between Physics and Graphics.
- [ ] **Portal Momentum Mechanics**
    - [ ] Transform velocity vectors correctly across portal planes.

## Phase 5: Developer Experience (Tools & UI) [DONE]
- [x] **Engine UI (Editor)** [DONE]
    - [x] Integrated `ImGui` (Docking branch).
    - [x] Scene hierarchy viewer and property editing.
- [/] **Workflow & Lifecycle** [/]
    - [ ] Play/Stop simulation logic.
    - [ ] Dynamic Mouse Locking (ESC to release).
    - [ ] Asset browser foundation.
    - [ ] Model loader (supporting modern formats like `glTF`).
    - [ ] Material system description.

## Phase 6: Game Logic & Recreations
- [ ] Implement Player Controller (Source-like movement).
- [ ] Modular Weapon/Tool system (The Portal Gun).
- [ ] Level loading from standard formats.
