# Game Engine Build System Architecture

## Overview

Modern game engines don't just copy project files into an executable. They follow a sophisticated multi-stage compilation and packaging pipeline that:

1. **Compiles** source code (C++, shaders, scripts)
2. **Processes** assets (meshes, textures, audio, materials)
3. **Optimizes** content for distribution
4. **Packages** everything into sealed, non-editable containers
5. **Links** the executable with all dependencies

This document explains how industry-standard engines (Unity, Unreal Engine, Source Engine) accomplish this, serving as a blueprint for implementing a similar system in your custom OpenGL C++ engine.

---

## Stage 1: Project Compilation & Build System

### How Unreal Engine Does It

Unreal uses **UnrealBuildTool (UBT)**, a custom C# build system that:

- Reads `.build.cs` and `.target.cs` configuration files defining modules, dependencies, and build rules
- Compiles C++ game code separately from engine code
- Links against pre-compiled engine libraries
- Handles platform-specific compilation (Win64, Mac, Linux)
- Supports multiple **build configurations**:
  - **Debug**: No optimizations, full debugging symbols
  - **Development**: Optimizations enabled, debuggable
  - **Shipping**: Maximum optimization, no debug info

### How Source Engine Does It

Source (used by CS2, Portal, Portal 2, TF2):
- Uses **VBSPc** (map compiler) for level geometry
- **ResourceCompiler** CLI utility compiles individual assets (models, textures, materials, sounds)
- Generates optimized, binary output files
- Maps reference compiled resources, not source files

### What Your Engine Should Do

Create a **Build Configuration System**:

```
Config Files (.cfg or .json)
├─ Project settings (entry point, target platforms)
├─ Module dependencies (which .libs, .dlls needed)
├─ Asset paths (where raw content is stored)
└─ Build rules (optimization levels, platform targets)

Build Tool (executable or CLI)
├─ Parse configuration
├─ Invoke C++ compiler
├─ Link with required libraries
├─ Validate project integrity
└─ Output distributable executable
```

---

## Stage 2: Asset Processing & Compilation

This is **critical for preventing modification**. Raw assets must be compiled into sealed formats.

### How Unreal Engine Does It

- Uses **Cooked** assets in platform-specific formats
- Raw assets (FBX, TGA, WAV) are processed by asset cookers
- Cookers generate optimized binary files (.uasset format)
- Stored in `Saved/Cooked/[Platform]` directories
- Final game bundles only cooked assets, not source

### How Source 2 Does It (CS2, Newer Source Games)

**Two-sided asset structure**:

| Side | Purpose | Format | Editable? |
|------|---------|--------|-----------|
| **Content Side** | Authoring, editing, source data | ASCII/text, source files (VMF, VTF, MDL) | ✅ Yes |
| **Game Side** | Runtime, compiled assets | Binary, compiled resources | ❌ No |

**ResourceCompiler** system:
- Processes content-side files
- Outputs compiled game-side resources
- Engine cannot use content-side files directly
- Like source code → machine code analogy

### Asset Categories to Compile

Each requires different processing:

#### **Meshes/Models**
- **Input**: FBX, OBJ, proprietary mesh formats
- **Process**: 
  - Load geometry data
  - Optimize vertex/index buffers
  - Bake tangent space
  - Validate bounds
- **Output**: Binary format (proprietary binary, GLB, or custom)
- **Example**: Unreal's .uasset, Source's .mdl

#### **Textures**
- **Input**: PNG, TGA, EXR, JPG
- **Process**:
  - Convert to optimal hardware format (BC1-BC7 compression, DXT)
  - Generate mipmaps
  - Validate dimensions (power-of-2 often required)
  - Compress for distribution
- **Output**: Binary texture format with metadata
- **Example**: Unreal's .uasset, Source's .vtf (Valve Texture Format)

#### **Audio**
- **Input**: WAV, MP3, FLAC
- **Process**:
  - Normalize levels
  - Resample to target rate
  - Compress with codec (Opus, Vorbis)
  - Validate channels (mono/stereo)
- **Output**: Binary audio file with metadata
- **Example**: Compiled audio packages in asset containers

#### **Materials/Shaders**
- **Input**: Material definition files (human-readable)
- **Process**:
  - Parse material properties
  - Compile shader source to bytecode
  - Link with texture references
  - Optimize for target GPU
- **Output**: Binary material + compiled shader objects
- **Example**: Source's .vmat + compiled shader bytecode

#### **Levels/Maps**
- **Input**: Editor map files (VMF for Source, umap for Unreal)
- **Process**:
  - Validate entity references
  - Compile visibility (VIS data for occlusion culling)
  - Compile BSP/geometry
  - Bake lightmaps
  - Compile scripts/logic
- **Output**: Binary map format, fully compiled
- **Example**: Source's .bsp files, Unreal's .umap (cooked)

---

## Stage 3: Asset Bundling & Packaging

Raw compiled assets must be packaged into sealed containers that prevent modification.

### How Unreal Engine Does It

**Project Packaging Process**:

1. Creates release build structure:
   ```
   Build Output/
   ├─ Binaries/
   │  └─ Win64/
   │     └─ UE4Game.exe (or renamed)
   ├─ Content/ (optional, if cooking disabled)
   ├─ Saved/
   │  └─ Cooked/
   │     └─ WindowsNoEditor/
   │        └─ [All compiled assets]
   └─ Engine/
      └─ Binaries/
         └─ Libraries and DLLs
   ```

2. Asset organization:
   - All cooked assets in platform-specific directories
   - Can be further packed into `.pak` files (sealed archives)
   - Multiple pak files for organization or patching

### How O3DE (Open 3D Engine) Does It

Uses **PAK format** (sealed asset containers):

1. **Asset Processing**:
   - Scan project for all assets
   - Compile each asset type
   - Generate `.assetlist` files (asset inventory)

2. **Bundling**:
   - Create `engine.pak` (engine resources)
   - Create `game.pak` (game-specific assets)
   - PAK files are binary archives, not extractable by players
   - Version numbering: `game_v1.pak`, `game_v2.pak` for patching

3. **Distribution**:
   ```
   Final Build/
   ├─ Launcher.exe
   ├─ Libraries (DLLs)
   └─ Cache/
      ├─ game_v1.pak
      └─ engine_v1.pak
   ```

### How Source Engine Does It

Uses **VPK format** (Valve Pak):

- Binary package format (not text-readable)
- Contains compressed, compiled resources
- Mount point system via `gameinfo.txt`
- Game can mount multiple VPK files
- Content structure example:
  ```
  game_content.vpk
  ├─ maps/
  │  └─ map001.bsp (compiled)
  ├─ models/
  │  └─ character.mdl (compiled)
  ├─ materials/
  │  └─ textures/
  │     └─ wall.vtf (compiled)
  └─ sounds/
     └─ ambient.mp3 (compressed)
  ```

---

## Stage 4: Executable Building & Linking

The final executable must be project-specific and link to all compiled assets.

### Key Principles

**The executable is NOT generic**:
- Simply swapping executables between two games doesn't work (even for same engine)
- The executable must know:
  - Where to find compiled assets
  - What the entry point/startup scene is
  - Which game modules to load
  - Configuration settings specific to the project

### How This Works

**Build Time**:

1. **Compile C++ Code**:
   ```cpp
   // Project-specific C++ compiled to .obj files
   // Then linked into .exe
   ```

2. **Embed Configuration**:
   ```cpp
   // Baked into executable or stored alongside it
   const char* GAME_NAME = "MyGame";
   const char* ENTRY_LEVEL = "levels/level_01.bsp";
   const char* ASSET_PATH = "./Assets/";
   ```

3. **Link Everything**:
   - Engine code (precompiled libraries)
   - Game code (newly compiled)
   - Runtime dependencies (DirectX, OpenGL, SDL, etc.)
   - Output: `MyGame.exe` (project-specific executable)

### What You Need in Your Custom Engine

**Build Process**:

```
Step 1: Read project config (JSON/YAML)
Step 2: Compile game C++ code → .obj files
Step 3: Link .obj + engine libs + dependencies → .exe
Step 4: Copy executable to output folder
Step 5: Process all assets (textures, meshes, audio, etc.)
Step 6: Package compiled assets → pak/container file
Step 7: Copy pak files alongside .exe
Step 8: Generate launcher script/shortcut
```

---

## Stage 5: Protection Against Modification

This is why bundling and compilation matters.

### What Players Receive

```
MyGame/
├─ MyGame.exe (native binary, not readable)
├─ engine.dll (library, not readable)
├─ assets.pak (binary archive, locked)
└─ config.ini (potentially readable, but not asset definitions)
```

### Why Files Can't Be Modified

1. **Compiled Assets**:
   - Textures are in proprietary binary format (not PNG)
   - Models are in binary format (not FBX)
   - Materials are compiled bytecode (not text)
   - Maps are fully compiled (not VMF/umap editor format)

2. **Sealed Containers**:
   - Assets in `.pak`/`.vpk` files (binary archives)
   - Not extractable by standard tools
   - Custom loader needed to read them
   - Player cannot swap files without specialized tools

3. **Runtime Validation**:
   - Checksums on asset files
   - Version validation in pak file headers
   - Game won't load mismatched or tampered assets

4. **No Editor Included**:
   - Game executable is launcher only
   - Editor tool separate from game distribution
   - No ability to modify without recompile

### Comparison: Mod Support (Optional)

If you want to allow modding:

```
Official Game Structure:
├─ MyGame.exe
└─ official_assets.pak (sealed)

Mod Support:
├─ MyGame.exe
├─ official_assets.pak (sealed)
└─ mods/ (user folder)
   └─ cool_mod.pak (community created)
```

The game engine can load additional pak files in specific directories, allowing modding without compromising the base game.

---

## Implementation Blueprint for Your Engine

### 1. Asset Pipeline

Create processors for each asset type:

```cpp
class AssetProcessor {
public:
    // Texture processing
    void ProcessTexture(const std::string& input_path, 
                       const std::string& output_path,
                       TextureFormat format);
    
    // Mesh processing
    void ProcessMesh(const std::string& input_path,
                    const std::string& output_path);
    
    // Shader compilation
    void CompileShader(const std::string& glsl_source,
                      const std::string& output_path);
    
    // Level compilation
    void CompileLevel(const std::string& map_file,
                     const std::string& output_path);
};
```

### 2. Asset Container Format

Define your own `.pak`-like format:

```cpp
struct PakHeader {
    uint32_t magic = 0x504B4147;  // "PKAG"
    uint32_t version = 1;
    uint32_t file_count;
    uint32_t toc_offset;  // Table of contents
    uint32_t checksum;    // For integrity checking
};

struct PakFileEntry {
    char filename[256];
    uint32_t offset;
    uint32_t size;
    uint32_t compressed_size;
    uint32_t flags;  // Encrypted? Compressed?
};
```

### 3. Build Tool CLI

```bash
# Build for Windows
MyEngineBuilder.exe build --project MyGame.proj --config Release --platform Windows

# Package assets
MyEngineBuilder.exe package --input ./Content --output ./Build/assets.pak

# Full build
MyEngineBuilder.exe build-all --project MyGame.proj --output ./Dist
```

### 4. Launcher Executable

Keep actual game executable minimal:

```cpp
int main(int argc, char** argv) {
    // Load engine configuration
    GameConfig config = LoadConfig("game.cfg");
    
    // Initialize engine
    Engine engine(config);
    
    // Mount asset containers
    engine.MountPak("assets.pak");
    engine.MountPak("audio.pak");
    
    // Load entry level
    Level* level = engine.LoadLevel(config.entry_level);
    
    // Main loop
    while (engine.IsRunning()) {
        engine.Update();
        engine.Render();
    }
    
    return 0;
}
```

---

## Reference: How Major Engines Do It

### Unity Build Pipeline

1. **Compilation**: Compiles all C# code + native plugins
2. **Cooking**: Processes all assets into optimized formats
3. **Packaging**: 
   - Creates executable stub
   - Builds asset database
   - Generates scenes and resources
4. **Distribution**:
   - Single .exe + data folder
   - Data is binary, not source format

### Unreal Engine 5

1. **Module Compilation**: Via UnrealBuildTool (UBT)
   - Compiles game modules separately
   - Links against engine libraries
2. **Asset Cooking**: Processes all content
   - Platform-specific (.uasset format)
   - Optimal compression
3. **Packaging**: 
   - Game executable
   - Cooked content folders/pak files
   - Required DLLs

### Source 2 (CS2, Newer Valve Games)

1. **Content Authoring**: VMAP files (Hammer editor)
2. **ResourceCompiler**: Converts all sources to compiled binary
3. **VPK Packaging**: Creates sealed asset containers
4. **Engine**: Runtime loads only compiled VPK files
5. **Protection**: Content-side never distributed

---

## Key Takeaways

1. **Separate Content & Game**:
   - Content side: Editable source files (for your team)
   - Game side: Compiled binary files (for players)

2. **Automate Everything**:
   - Build tool automates compilation, cooking, packaging
   - No manual asset organization before distribution

3. **Seal Asset Containers**:
   - Use binary archive format (.pak)
   - Players cannot easily extract/modify
   - Integrity checking prevents tampering

4. **Project-Specific Executables**:
   - Each game build gets unique executable
   - Cannot swap exes between different projects
   - Configuration baked in

5. **Asset Compilation is Code Compilation**:
   - Textures → optimized binary
   - Models → processed binary
   - Maps → compiled binary
   - Same principle as C++ → machine code

6. **Scalable for Patching**:
   - Version-numbered pak files (game_v1.pak, game_v2.pak)
   - Players download only changed versions
   - New pak files replace old ones

---

## Next Steps for Implementation

1. **Design asset processors** for each resource type
2. **Define pak file format** with compression and integrity checking
3. **Build automation tool** (command-line, C++ or C#)
4. **Create minimal launcher** that loads pak files and runs game
5. **Test distribution**: Ensure players cannot modify assets
6. **Implement versioning**: Plan for updates and patches

This approach mirrors industry standards and provides both security (against casual modification) and flexibility (for future expansion and modding support if desired).
