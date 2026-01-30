# Source67 Game Engine Build System – Full Guide for AI Assistant

## Goal

You are helping build a custom C++ OpenGL 3D engine that can:

- Build **standalone games** (like Source, Unity, Unreal).
- Produce proper **.exe**, **.dll** (or .so/.dylib), asset containers, and runtime folders.
- Separate **editor** from **runtime**.
- Prevent players from casually editing levels, textures, audio, etc., while still allowing optional mod support later.

Your job is to design and reason about the **entire build pipeline**, not just asset packaging.

---

## High-Level Build Pipeline

All mature engines follow a similar multi-stage pipeline:

1. **Source compilation**
   - Compile engine C++ → **engine library + engine executable**.
   - Compile game C++ / scripts → **game module or game executable/DLL**.

2. **Asset cooking (content → game data)**
   - Convert editable sources (FBX, PNG, WAV, JSON scenes) into **binary, optimized formats**.
   - Make runtime read only cooked formats, not editor sources.

3. **Asset packaging**
   - Bundle cooked assets into archives (`.pak`, `.vpk`, `.assetpack`, `data.unity3d`, etc.).
   - Or into structured “\_Data” folders (Unity style).

4. **Final layout and distribution**
   - Create per-platform output folder containing:
     - **Game.exe** (or platform equivalent).
     - Engine and third-party **.dll/.so/.dylib**.
     - Asset containers / data folders.
     - Config files and optional launcher/installer.

5. **Protection / integrity**
   - Don’t ship editor sources.
   - Use binary formats and containers.
   - Optional checksums/versioning to detect tampering.

---

## 1. Code Structure: Engine, Game, Runtime vs Editor

### Common Patterns

Different engines structure code in different ways, but the patterns are similar:

- **Unity**
  - Engine runtime is native C++ inside a player executable (e.g., UnityPlayer).
  - User code (C# scripts) compiled to assemblies, loaded by the runtime.
  - Build outputs: `MyGame.exe` + `MyGame_Data` folder with serialized data and managers.

- **Unreal Engine**
  - Engine: large C++ codebase compiled into many **engine DLLs** and static libs.
  - Game: C++ modules compiled via a build tool and linked to create a **project-specific .exe** plus DLLs.
  - Editor logic compiled separately; shipping build excludes editor-only code.

- **Custom engine example (C++ + game DLL)**
  - Engine compiled into **Engine.exe**.
  - Game compiled into **Game.dll**, loaded at runtime.
  - Engine finds and loads `Game.dll`, calls exported entry points, and provides an API table.

### Options for Your Engine

Design one of these architectures:

1. **Monolithic executable**
   - Engine and game compiled into **one .exe**.
   - Simple linking: engine source + game source → `Game.exe`.
   - Easier initially, harder to support plugins/mods.

2. **Engine exe + game DLL**
   - Engine → `EngineRuntime.exe`.
   - Game → `MyGame.dll` exposing something like `GameMain(EngineAPI*)`.
   - Engine loads DLL via `LoadLibrary`/`dlopen`, passes function table.
   - Easier to support modding and multiple games on same engine.

3. **Launcher exe + engine DLL(s) + game DLL**
   - `Launcher.exe` is tiny: sets up environment, loads engine DLL and game DLL.
   - Engine core in `Engine.dll`; game in `MyGame.dll`.
   - Useful if you want multiple frontends (editor vs game) sharing the same engine core.

For this project, aim for **(2) or (3)**.

---

## 2. Building the Executable(s) and DLL(s)

### 2.1 Build Configurations

Support standard configurations:

- **Debug**: no optimizations, full symbols.
- **Development**: optimizations, still debuggable.
- **Shipping/Release**: heavy optimization, no debug data, editor code stripped.

Store config in project files (JSON/TOML/YAML) similar to common engine project settings.

### 2.2 Engine Build

- Compile engine C++ into:
  - **Static library** (e.g., `EngineCore.lib`) linked into game exe, or
  - **Dynamic library** (e.g., `EngineCore.dll`) loaded at runtime.
- Separate **editor-only** code into separate modules or build flags so shipping build does not include editor UI, debug tools, etc.

### 2.3 Game Build

Two common strategies:

- **Game as executable**
  - Link engine static libs + game code into `MyGame.exe`.
  - Externally link engine third-party DLLs (OpenGL loader, audio libs, etc.).

- **Game as DLL**
  - Engine runtime builds as `Engine.exe`.
  - Game builds as `Game.dll` with known exported functions (`GameInitialize`, `GameUpdate`, `GameShutdown`).
  - `Engine.exe` loads `Game.dll`, obtains entry points with `GetProcAddress`/`dlsym`, and drives the game loop.

The build system must:

- Know **include paths**, **library paths**, and **link order**.
- Produce per-platform binaries (Win64 exe, Linux ELF, etc.).
- Copy necessary third-party DLLs to the output directory next to the main exe.

---

## 3. Asset Cooking: From Editable to Runtime-Only

### 3.1 Concept: Content vs Game Side

Use the **content vs game** split:

- **Content side** (editor):
  - FBX/OBJ, PNG/TGA, WAV, JSON/scene files, level sources.
  - Editable by devs, not shipped to players.

- **Game side** (runtime):
  - Proprietary **binary formats** and containers.
  - Only these are shipped.

The runtime in shipping builds must never load content-side formats directly.

### 3.2 Per-Asset-Type Cooking

Implement cookers for each category:

- **Meshes**
  - Input: FBX/OBJ.
  - Process: optimize, generate tangents, LODs, bounds.
  - Output: binary vertex/index buffers in your own format.

- **Textures**
  - Input: PNG/TGA/EXR.
  - Process: convert to GPU-friendly compressed formats (BC/DXT/ASTC), generate mipmaps.
  - Output: binary texture format with metadata.

- **Audio**
  - Input: WAV/FLAC.
  - Process: normalize, resample, compress (Ogg/Opus/ADPCM).
  - Output: binary audio format with metadata.

- **Materials & shaders**
  - Input: material graphs or text-based definitions.
  - Process: compile shader source to bytecode, pack material parameters.
  - Output: binary material + shader bytecode files.

- **Levels/Scenes**
  - Input: editor level format (JSON/custom/VMF-like).
  - Process: build spatial structures, bake lighting, validate entity references.
  - Output: compiled binary level references only to cooked assets.

Runtime code must assume **cooked formats only** for shipping.

---

## 4. Packaging: Data Folders, Paks, VPK-like Archives

### 4.1 Folder-Based Layout (Unity-Style)

Unity builds: `[GameName].exe` + `[GameName]_Data` folder containing serialized resources and asset databases.

Example layout:

```

MyGame/
├─ MyGame.exe
├─ MyGame_Data/
│ ├─ globalgamemanagers
│ ├─ sharedassets0.assets
│ ├─ level0
│ ├─ resources.assets
│ └─ ...
└─ UnityPlayer-like runtime dll

```

Your engine can mimic this idea:

- `MyGame.exe`
- `MyGame_Data/` or `Content/` or `Assets/` containing cooked binaries or container files.

### 4.2 Archive-Based Layout (Pak/VPK/Assetpack)

Archive-based designs:

- `.pak` files holding cooked assets.
- `.vpk` files (Valve/Source-style) for compiled materials, models, maps, etc.
- Single or multiple `data`/`assetpack` files like many engines do.

Your container format should:

- Have a **header** (magic, version, flags, checksum).
- Have a **table of contents** (file path → offset, size, compression flags).
- Optionally compress/encrypt data.
- Support multiple packs: `engine.pak`, `game.pak`, `audio.pak`, DLC packs, etc.

At runtime, the engine mounts these packs and performs all asset I/O through them instead of raw filesystem access.

---

## 5. Final Output Layout (Per-Platform)

Design a standard distribution folder.

### Example: Engine exe + game DLL + Paks

```text
MyGame/
├─ MyGame.exe          # Launcher / engine runtime
├─ EngineCore.dll      # Engine core
├─ Game.dll            # Game logic module
├─ thirdparty/
│  ├─ OpenAL32.dll
│  └─ SDL2.dll
├─ Content/
│  ├─ engine.pak
│  ├─ game.pak
│  └─ audio.pak
└─ config/
   ├─ game.cfg
   └─ graphics.cfg
```

### Example: Unity-like Data Folder

```text
MyGame/
├─ MyGame.exe
├─ EngineRuntime.dll
└─ MyGame_Data/
   ├─ data.bin or data.unity3d style packs
   ├─ resources.assets
   └─ other cooked content
```

Requirements:

- Exe and DLLs are placed so the OS can locate them.
- Data folder or containers are located at known relative paths or configured in a config file.

---

## 6. Protection Against Casual Modification

To avoid users freely editing textures, levels, audio, etc.:

1. **Do not ship source assets**
   - No FBX, PSD, raw PNG, WAV, or text scene files in the final build.
2. **Use binary-only runtime formats**
   - Custom binary layouts for textures, meshes, levels, materials, etc.
   - Keep formats internal.
3. **Use asset containers**
   - Store all runtime assets in `.pak`/`.vpk`-like containers or `.assetpack` files.
   - Optionally add compression/encryption.
4. **Validate at runtime**
   - Check checksums and version in pack headers.
   - Refuse to load corrupted or mismatched packs.
5. **Exclude editor**
   - Never distribute the editor exe or tools with the game.
   - Runtime cannot modify or recook content; it only reads cooked data.

You can later support modding by defining a `mods/` folder and loading extra packs from there while keeping official content sealed.

---

## 7. Build Tool / CLI Design

Create a **build tool** (CLI) that orchestrates the entire pipeline.

High-level commands:

- `build-engine`
  - Compiles engine core and editor/runtimes.
- `build-game`
  - Compiles game modules (.exe or .dll) for a given configuration/platform.
- `cook-assets`
  - Scans project content folders.
  - Runs asset cookers per type.
  - Outputs cooked binaries to an intermediate folder (`Cooked/Platform/`).
- `pack-assets`
  - Reads cooked assets and builds `.pak`/`.assetpack` files plus a TOC.
- `build-distribution`
  - Assembles final folder: copies exe, DLLs, containers, configs into `Dist/`.

Example CLI flow:

```bash
# Full shipping build for Windows

engine_build.exe \
  --project MyGame.proj \
  --platform Win64 \
  --config Shipping \
  --cook-assets \
  --pack-assets \
  --output ./Dist/Win64
```

---

## 8. Editor vs Standalone Runtime

Separate **editor** and **standalone game**:

- **Editor build**
  - Contains editor UI, tools for importing assets, placing entities, etc.
  - Works directly with content-side formats and can re-cook assets.
- **Standalone build**
  - Only engine runtime and game logic.
  - Loads only cooked assets from packs or data folders.
  - No editing features, smaller and optimized.

Workflow:

1. Developers use the editor to modify content-side assets.
2. Build tool cooks assets and builds game binaries.
3. Final output is the standalone build folder ready for distribution.

---

## 9. Design Targets for the AI

When you generate or refine this engine’s build system, aim for:

- Clear **separation** between:
  - Engine core vs game code.
  - Editor vs runtime.
- A robust **build pipeline**:
  - Compile → cook → pack → layout → ship.
- **Project-based configuration**:
  - Project file describing entry level, target platforms, build configs, asset paths.
- **Non-editable shipped assets**:
  - Only cooked binary formats + containers are distributed.
- Extensibility:
  - Support DLC and mod packs by mounting extra containers.
  - Allow per-platform builds (Windows, Linux, etc.) with appropriate binaries.

Focus on:

- How to produce `.exe` and `.dll` files for the game and engine.
- How the runtime discovers and loads game DLLs and asset containers.
- How to design the on-disk layout to look and behave like “real” commercial engine builds.

This document is your guide for designing and implementing the full build system for the engine.

```

```
