# Game Engine Build System Specification

## Overview

This document specifies a complete build system for an OpenGL/C++20 3D game engine that separates the engine runtime from game code and assets using a DLL-based architecture with asset packing.

**Architecture Pattern:**

- Game code compiles to a single `Game.dll` (dynamic library)
- Assets pack into an `Assetpack` binary file (scenes, models, textures, sounds, etc.)
- Engine builds into a standalone executable that loads both at runtime
- Editor mode includes debug helpers that are stripped in standalone builds

---

## Build Pipeline Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                        BUILD PROCESS                            │
└─────────────────────────────────────────────────────────────────┘

[Game Source Code]  [Asset Files]         [Engine Source Code]
        │                  │                        │
        │                  │                        │
        ▼                  ▼                        ▼
    ┌────────────┐    ┌─────────────┐         ┌─────────────┐
    │C++20 Compiler│    │Asset Packer │         │C++20 Compiler
    │             │    │  (Custom)   │         │             │
    └────────────┘    └─────────────┘         └─────────────┘
        │                  │                        │
        ▼                  ▼                        ▼
   [Game.dll]      [Assetpack.bin]          [Engine.exe]
        │                  │                        │
        └──────────────────┴────────────────────────┘
                          │
                          ▼
                    ┌─────────────────┐
                    │ Runtime Loader  │
                    │ (Engine.exe)    │
                    └─────────────────┘
                          │
            ┌─────────────┼─────────────┐
            │             │             │
            ▼             ▼             ▼
      Load DLL    Load Assetpack   Initialize
                                    OpenGL/Graphics
                          │
                          ▼
                   ┌────────────────┐
                   │ Running Game   │
                   └────────────────┘
```

---

## Part 1: Game Code Compilation (Game.dll)

### 1.1 Game Source Structure

```
game/
├── src/
│   ├── main.cpp              # Game entry point
│   ├── game_state.h
│   ├── game_state.cpp
│   ├── player.h
│   ├── player.cpp
│   ├── world.h
│   ├── world.cpp
│   ├── game_api.h            # API exposed to engine
│   └── game_api.cpp
├── CMakeLists.txt            # Or build script
└── build/                     # Output directory
    └── Release/
        └── Game.dll          # Compiled output
```

### 1.2 Game API Interface

The game code must expose a C-compatible interface (`game_api.h`) with these core functions:

```cpp
// game_api.h - Exported functions for engine to call

extern "C" {
    // Initialization
    void game_initialize(void* engine_context);

    // Update loop
    void game_update(float delta_time);
    void game_render();

    // Input handling
    void game_on_key_pressed(int key_code);
    void game_on_key_released(int key_code);
    void game_on_mouse_moved(float x, float y);

    // Asset loading (engine calls this when Assetpack is loaded)
    void game_on_assets_loaded(void* assetpack_handle);

    // Shutdown
    void game_shutdown();

    // Version/metadata
    const char* game_get_version();
    int game_get_build_number();
}
```

### 1.3 Compilation Process

**Release Mode Build:**

```bash
# Using CMake example (or your build system)
cd game/
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build --config Release

# Output: game/build/Release/Game.dll
# On Linux/Mac: game/build/Release/libGame.so or libGame.dylib
```

**Compilation Flags:**

- Optimize for performance (`-O3` or `/O2`)
- Strip debug symbols (can be optional for post-mortem debugging)
- Position-independent code (for DLL loading): `-fPIC`
- Export symbols: Add `__declspec(dllexport)` (Windows) or visibility attributes (Linux)

### 1.4 Game DLL Exports

The DLL must explicitly export the API functions:

```cpp
// game_api.cpp
#ifdef _WIN32
    #define GAME_API __declspec(dllexport)
#else
    #define GAME_API __attribute__((visibility("default")))
#endif

GAME_API void game_initialize(void* engine_context) { /* ... */ }
GAME_API void game_update(float delta_time) { /* ... */ }
// ... etc
```

---

## Part 2: Asset Packing (Assetpack.bin)

### 2.1 Asset File Structure

```
assets/
├── scenes/
│   ├── main_menu.scene
│   ├── level_01.scene
│   └── level_02.scene
├── models/
│   ├── player.fbx
│   ├── enemy.fbx
│   └── building.obj
├── textures/
│   ├── player_diffuse.png
│   ├── terrain.png
│   └── ui_atlas.png
├── sounds/
│   ├── bgm_main.ogg
│   ├── sfx_jump.wav
│   └── voice_intro.ogg
├── scripts/
│   ├── player_controller.gd
│   └── enemy_ai.gd
└── config.json               # Asset metadata
```

### 2.2 Asset Packer Implementation

The asset packer is a custom tool that reads all assets and bundles them into a single binary file.

**Assetpack Binary Format:**

```
[ASSETPACK HEADER]
├── Magic Number: "APAK" (4 bytes)
├── Version: 1 (4 bytes)
├── Total Asset Count: N (4 bytes)
├── Index Table Offset: (4 bytes)
├── Reserved: (8 bytes)
│
[ASSET DATA SECTION]
├── Asset 1 Data (raw bytes)
├── Asset 2 Data (raw bytes)
├── ... N Assets ...
│
[INDEX TABLE]
├── Asset Entry 1
│   ├── Asset ID Hash (8 bytes)
│   ├── Asset Type (4 bytes)
│   ├── Data Offset (8 bytes)
│   ├── Data Size (8 bytes)
│   ├── Compression Type (1 byte)
│   └── Path String (variable, null-terminated)
├── Asset Entry 2
│   └── ... same structure ...
├── ... N Asset Entries ...
│
[FOOTER]
├── Checksum/CRC (8 bytes)
└── Index Table Checksum (8 bytes)
```

### 2.3 Asset Packer Algorithm

```
INPUT: assets/ directory
OUTPUT: Assetpack.bin

1. Scan all asset files recursively
2. For each file:
   a. Calculate unique ID (hash of relative path)
   b. Determine asset type (model, texture, sound, scene, etc.)
   c. Apply compression if beneficial (DEFLATE or LZ4)
   d. Record metadata (type, offset, size, path)
3. Write header with metadata
4. Write all asset data sequentially
5. Write index table (sorted by asset ID for fast lookup)
6. Calculate checksums
7. Write footer
8. Output: Assetpack.bin
```

### 2.4 Asset Type Enumeration

```cpp
enum AssetType : uint32_t {
    ASSET_UNKNOWN = 0,
    ASSET_TEXTURE = 1,
    ASSET_MODEL = 2,
    ASSET_SOUND = 3,
    ASSET_SCENE = 4,
    ASSET_SCRIPT = 5,
    ASSET_CONFIG = 6,
    ASSET_SHADER = 7,
    ASSET_FONT = 8,
    // ... add as needed
};
```

### 2.5 Packer Tool Command

```bash
# Command-line interface
./asset_packer -i assets/ -o Assetpack.bin -c lz4 -v

# Options:
# -i, --input <dir>      Input assets directory
# -o, --output <file>    Output Assetpack filename
# -c, --compression <type>  Compression algorithm (none, deflate, lz4)
# -v, --verbose          Verbose output
# --validate             Validate integrity after packing
```

---

## Part 3: Engine Runtime (Engine.exe)

### 3.1 Engine Startup Sequence

When `Engine.exe` launches, it performs these steps in order:

```cpp
int main(int argc, char* argv[]) {
    try {
        // Step 1: Initialize window and graphics context
        Engine engine;
        engine.initialize_graphics();

        // Step 2: Search for Game.dll and Assetpack.bin
        std::string game_dll_path = engine.find_game_dll();
        std::string assetpack_path = engine.find_assetpack();

        if (game_dll_path.empty() || assetpack_path.empty()) {
            throw std::runtime_error("Game DLL or Assetpack not found!");
        }

        // Step 3: Load Assetpack into memory
        void* assetpack_handle = engine.load_assetpack(assetpack_path);

        // Step 4: Load Game.dll dynamically
        void* game_dll_handle = engine.load_game_dll(game_dll_path);

        // Step 5: Resolve game API functions from DLL
        GameAPI game_api = engine.resolve_game_api(game_dll_handle);

        // Step 6: Initialize game with engine context
        game_api.game_initialize(&engine);

        // Step 7: Notify game that assets are ready
        game_api.game_on_assets_loaded(assetpack_handle);

        // Step 8: Main loop
        while (engine.is_running()) {
            float delta_time = engine.calculate_delta_time();

            // Handle input and pass to game
            engine.handle_input();

            // Update game logic
            game_api.game_update(delta_time);

            // Render frame
            game_api.game_render();

            engine.swap_buffers();
        }

        // Step 9: Cleanup
        game_api.game_shutdown();
        engine.unload_game_dll(game_dll_handle);
        engine.unload_assetpack(assetpack_handle);
        engine.shutdown();

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Engine error: " << e.what() << std::endl;
        return 1;
    }
}
```

### 3.2 Finding Game Assets

The engine searches for DLL and Assetpack in this order:

```cpp
std::string Engine::find_game_dll() {
    // Search locations (in order):
    // 1. Same directory as Engine.exe
    // 2. ./game/ subdirectory
    // 3. ../game/build/Release/ (relative to exe)
    // 4. Environment variable GAME_DLL_PATH
    // 5. Current working directory

    std::vector<std::string> search_paths = {
        "./Game.dll",
        "./game/Game.dll",
        "../game/build/Release/Game.dll",
        std::getenv("GAME_DLL_PATH") ? std::getenv("GAME_DLL_PATH") : "",
    };

    for (const auto& path : search_paths) {
        if (std::filesystem::exists(path)) {
            return std::filesystem::absolute(path).string();
        }
    }

    return "";  // Not found
}

std::string Engine::find_assetpack() {
    // Similar search for Assetpack.bin
    std::vector<std::string> search_paths = {
        "./Assetpack.bin",
        "./assets/Assetpack.bin",
        "../assets/Assetpack.bin",
        std::getenv("ASSETPACK_PATH") ? std::getenv("ASSETPACK_PATH") : "",
    };

    for (const auto& path : search_paths) {
        if (std::filesystem::exists(path)) {
            return std::filesystem::absolute(path).string();
        }
    }

    return "";  // Not found
}
```

### 3.3 Dynamic DLL Loading

```cpp
void* Engine::load_game_dll(const std::string& dll_path) {
    #ifdef _WIN32
        HMODULE dll = LoadLibraryA(dll_path.c_str());
        if (!dll) {
            throw std::runtime_error(
                "Failed to load Game.dll: " + dll_path +
                " (Error: " + std::to_string(GetLastError()) + ")"
            );
        }
        return (void*)dll;
    #else
        void* dll = dlopen(dll_path.c_str(), RTLD_LAZY);
        if (!dll) {
            throw std::runtime_error(
                "Failed to load Game.dll: " + dll_path +
                " (" + std::string(dlerror()) + ")"
            );
        }
        return dll;
    #endif
}

void Engine::unload_game_dll(void* dll_handle) {
    #ifdef _WIN32
        FreeLibrary((HMODULE)dll_handle);
    #else
        dlclose(dll_handle);
    #endif
}
```

### 3.4 API Function Resolution

```cpp
GameAPI Engine::resolve_game_api(void* dll_handle) {
    GameAPI api = {};

    auto resolve_function = [dll_handle](const char* name) -> void* {
        #ifdef _WIN32
            return (void*)GetProcAddress((HMODULE)dll_handle, name);
        #else
            return dlsym(dll_handle, name);
        #endif
    };

    api.game_initialize = (decltype(api.game_initialize))
        resolve_function("game_initialize");
    api.game_update = (decltype(api.game_update))
        resolve_function("game_update");
    api.game_render = (decltype(api.game_render))
        resolve_function("game_render");
    api.game_on_key_pressed = (decltype(api.game_on_key_pressed))
        resolve_function("game_on_key_pressed");
    api.game_on_key_released = (decltype(api.game_on_key_released))
        resolve_function("game_on_key_released");
    api.game_on_mouse_moved = (decltype(api.game_on_mouse_moved))
        resolve_function("game_on_mouse_moved");
    api.game_on_assets_loaded = (decltype(api.game_on_assets_loaded))
        resolve_function("game_on_assets_loaded");
    api.game_shutdown = (decltype(api.game_shutdown))
        resolve_function("game_shutdown");

    // Verify all functions are found
    if (!api.game_initialize || !api.game_update || !api.game_render) {
        throw std::runtime_error("Failed to resolve required game API functions");
    }

    return api;
}
```

### 3.5 Assetpack Loading

```cpp
void* Engine::load_assetpack(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open Assetpack: " + path);
    }

    // Read and validate header
    AssetpackHeader header;
    file.read((char*)&header, sizeof(header));

    if (std::memcmp(header.magic, "APAK", 4) != 0) {
        throw std::runtime_error("Invalid Assetpack magic number");
    }

    if (header.version != 1) {
        throw std::runtime_error("Unsupported Assetpack version");
    }

    // Allocate memory for entire assetpack
    AssetpackData* data = new AssetpackData();
    data->asset_count = header.asset_count;

    // Read all asset data into memory
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    data->memory = new uint8_t[file_size];
    file.read((char*)data->memory, file_size);
    file.close();

    // Read index table
    file.seekg(header.index_table_offset);
    data->index_table = new AssetEntry[header.asset_count];
    for (uint32_t i = 0; i < header.asset_count; ++i) {
        // Read entry (implementation depends on your format)
    }

    // Validate checksum
    uint64_t expected_checksum = *(uint64_t*)(data->memory + file_size - 16);
    uint64_t calculated = calculate_crc(data->memory, file_size - 8);
    if (expected_checksum != calculated) {
        throw std::runtime_error("Assetpack checksum failed");
    }

    return (void*)data;
}

void Engine::unload_assetpack(void* handle) {
    AssetpackData* data = (AssetpackData*)handle;
    delete[] data->memory;
    delete[] data->index_table;
    delete data;
}
```

### 3.6 Asset Retrieval API

The engine exposes functions for the game to access packed assets:

```cpp
// Called by game code to load an asset
void* Engine::get_asset(void* assetpack_handle, const char* asset_path) {
    AssetpackData* pack = (AssetpackData*)assetpack_handle;

    // Hash the asset path for fast lookup
    uint64_t asset_id = hash_string(asset_path);

    // Binary search in index table
    const AssetEntry* entry = pack->find_asset(asset_id);
    if (!entry) {
        return nullptr;  // Asset not found
    }

    // Return pointer to asset data in memory
    return (void*)(pack->memory + entry->data_offset);
}

size_t Engine::get_asset_size(void* assetpack_handle, const char* asset_path) {
    AssetpackData* pack = (AssetpackData*)assetpack_handle;
    uint64_t asset_id = hash_string(asset_path);

    const AssetEntry* entry = pack->find_asset(asset_id);
    return entry ? entry->data_size : 0;
}
```

---

## Part 4: Build Modes

### 4.1 Editor Mode

When running in **Editor Mode** (development/debugging):

```cpp
// In engine code
#ifdef EDITOR_MODE
    // Enable debug overlays
    engine.show_debug_ui();
    engine.show_asset_inspector();
    engine.show_performance_metrics();

    // Hot-reload capabilities
    engine.enable_hot_reload();

    // Detailed logging
    engine.set_log_level(LogLevel::DEBUG);
#endif
```

**Building for Editor Mode:**

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DEDITOR_MODE=ON -B build
cmake --build build --config Debug
```

### 4.2 Standalone Mode

When running in **Standalone Mode** (shipping/release):

```cpp
// In engine code
#ifdef STANDALONE_MODE
    // Remove all debug code
    #define EDITOR_DEBUG(x)  // No-op macro

    // Disable hot-reload
    engine.disable_hot_reload();

    // Minimal logging
    engine.set_log_level(LogLevel::ERROR);

    // Optimize for performance
    engine.enable_optimizations();
#endif
```

**Building for Standalone Mode:**

```bash
cmake -DCMAKE_BUILD_TYPE=Release -DSTANDALONE_MODE=ON -B build
cmake --build build --config Release
```

**Stripping Debug Helpers:**

```cpp
// Headers should have guard macros
#ifndef STANDALONE_MODE
    // This code is removed in standalone builds
    void debug_draw_bounding_boxes() { /* ... */ }
    void debug_show_entity_tree() { /* ... */ }
#endif
```

---

## Part 5: Complete Build Script

### 5.1 Master Build Script (`build.sh` or `build.bat`)

```bash
#!/bin/bash
# Complete build script for game engine + game + assets

set -e  # Exit on error

BUILD_TYPE=${1:-Release}
TARGET=${2:-all}

echo "========================================="
echo "Game Engine Build System"
echo "========================================="
echo "Build Type: $BUILD_TYPE"
echo "Target: $TARGET"
echo ""

# Build game code
if [ "$TARGET" = "all" ] || [ "$TARGET" = "game" ]; then
    echo "[1/3] Building Game.dll..."
    cd game
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -B build
    cmake --build build --config $BUILD_TYPE
    GAME_DLL="$(pwd)/build/$BUILD_TYPE/Game.dll"
    cd ..
    echo "✓ Game.dll built at: $GAME_DLL"
    echo ""
fi

# Pack assets
if [ "$TARGET" = "all" ] || [ "$TARGET" = "assets" ]; then
    echo "[2/3] Packing assets into Assetpack.bin..."
    ./asset_packer -i assets/ -o Assetpack.bin -c lz4 -v --validate
    echo "✓ Assetpack.bin created"
    echo ""
fi

# Build engine
if [ "$TARGET" = "all" ] || [ "$TARGET" = "engine" ]; then
    echo "[3/3] Building Engine.exe..."
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DSTANDALONE_MODE=ON -B build
    cmake --build build --config $BUILD_TYPE
    ENGINE_EXE="$(pwd)/build/$BUILD_TYPE/Engine.exe"
    cd ..
    echo "✓ Engine executable built at: $ENGINE_EXE"
    echo ""
fi

echo "========================================="
echo "Build Complete!"
echo "========================================="
echo ""
echo "Distribution package:"
echo "  - $ENGINE_EXE"
echo "  - Assetpack.bin"
echo "  - Game.dll"
echo ""
echo "To run: $ENGINE_EXE"
```

### 5.2 Build Targets

```makefile
# CMakeLists.txt (root)

cmake_minimum_required(VERSION 3.20)
project(GameEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(STANDALONE_MODE "Build in standalone mode (no editor features)" OFF)
option(EDITOR_MODE "Build in editor mode (with debug features)" ON)

# Subdirectories
add_subdirectory(engine)
add_subdirectory(game)

# Custom targets
add_custom_target(pack_assets
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/asset_packer
        -i ${CMAKE_CURRENT_SOURCE_DIR}/assets/
        -o ${CMAKE_BINARY_DIR}/Assetpack.bin
        -c lz4 -v
    COMMENT "Packing assets..."
)

add_custom_target(build_complete
    DEPENDS engine game pack_assets
    COMMENT "Full build complete"
)

# Run game
add_custom_target(run
    COMMAND ${CMAKE_BINARY_DIR}/engine/Engine
    DEPENDS build_complete
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
```

---

## Part 6: File Locations and Distribution

### 6.1 Final Distribution Structure

```
game_release/
├── Engine.exe              # Main executable (or Engine on Linux/Mac)
├── Assetpack.bin          # All game assets
├── Game.dll               # Game code (Windows)
├── libGame.so             # Game code (Linux)
├── libGame.dylib          # Game code (macOS)
└── README.txt             # Distribution notes
```

### 6.2 Runtime Requirements

**For the game to run, all three components must be present:**

1. **Engine.exe** - The runtime
2. **Game.dll** - Game logic
3. **Assetpack.bin** - Assets

If any are missing, the engine should provide a clear error message:

```
ERROR: Cannot find Game.dll
Searched in:
  - ./Game.dll
  - ./game/Game.dll
  - ../game/build/Release/Game.dll
```

---

## Part 7: Workflows

### 7.1 Development Workflow

```
1. Edit game source code in game/src/
2. Edit assets in assets/
3. Run: ./build.sh Debug all
4. Test in debug mode with editor features
5. Iterate until satisfied
```

### 7.2 Shipping Workflow

```
1. Final game code review
2. Ensure all assets are in assets/
3. Run: ./build.sh Release all
4. Verify Engine.exe + Game.dll + Assetpack.bin exist
5. Package as distribution
6. Ship!
```

---

## Part 8: Error Handling and Validation

### 8.1 Compilation Errors

**Game DLL Compilation Fails:**

```
Error: Game.dll failed to compile
- Check game/src/ for syntax errors
- Verify all headers are present
- Ensure game_api.h exports are correct
```

**Engine Compilation Fails:**

```
Error: Engine.exe failed to compile
- Check engine/ for syntax errors
- Verify OpenGL headers and libraries are linked
- Check CMake configuration
```

### 8.2 Runtime Errors

**DLL Not Found:**

```
ERROR: Game.dll not found at ./Game.dll
Searched in:
  - ./Game.dll
  - ./game/Game.dll
  - ../game/build/Release/Game.dll
  - $GAME_DLL_PATH environment variable
```

**Assetpack Corrupted:**

```
ERROR: Assetpack.bin checksum validation failed
File may be corrupted. Try rebuilding with: ./build.sh Release assets
```

**API Function Missing:**

```
ERROR: Failed to resolve game_initialize from Game.dll
Game DLL may be incompatible or outdated. Rebuild with matching engine version.
```

### 8.3 Validation Checklist

Before shipping, verify:

```
[ ] Game.dll compiles with no warnings
[ ] Assetpack.bin passes integrity check
[ ] Engine.exe starts without errors
[ ] Game initializes and loads assets
[ ] Frame rate is stable (60+ FPS)
[ ] All assets are accessible by the game
[ ] No memory leaks in profiler
[ ] Standalone mode builds successfully
[ ] Distribution package contains all three files
```

---

## Part 9: API Reference

### 9.1 Engine Context

The engine passes a context pointer to `game_initialize`:

```cpp
struct EngineContext {
    // Graphics
    void (*get_asset)(void* assetpack, const char* path);
    size_t (*get_asset_size)(void* assetpack, const char* path);

    // Input
    void (*set_key_callback)(void (*callback)(int key, int action));
    void (*set_mouse_callback)(void (*callback)(float x, float y));

    // Logging
    void (*log_info)(const char* message);
    void (*log_error)(const char* message);

    // Window
    int (*get_window_width)();
    int (*get_window_height)();
};
```

### 9.2 Assetpack Handle

Opaque handle passed to game:

```cpp
void game_on_assets_loaded(void* assetpack_handle) {
    // Use with engine->get_asset(assetpack_handle, "textures/player.png");
    // Store for later use in update/render
    g_assetpack = assetpack_handle;
}
```

---

## Part 10: Build System Implementation Checklist

For the AI agent implementing this system, follow in order:

- [ ] **Create game API header** (`game/src/game_api.h`)
  - Define C-compatible function signatures
  - Add export macros for DLL
- [ ] **Implement game API stubs** (`game/src/game_api.cpp`)
  - Implement all exported functions
  - Handle initialization, update, render, shutdown
- [ ] **Create asset packer tool** (`tools/asset_packer/`)
  - Parse asset directory
  - Implement binary format writer
  - Add compression support
  - Create CLI interface
- [ ] **Implement engine runtime** (`engine/src/runtime.cpp`)
  - DLL search and loading
  - Function resolution
  - Assetpack loading
- [ ] **Add engine context** (`engine/src/engine_context.h`)
  - Define EngineContext struct
  - Populate with function pointers
- [ ] **Create build scripts** (`build.sh`, `build.bat`)
  - Sequential build of game, assets, engine
  - Support for build modes
- [ ] **Update CMakeLists.txt**
  - Add game subdirectory
  - Add asset packing target
  - Add run target
- [ ] **Add error handling**
  - Comprehensive error messages
  - File existence checks
  - Validation of all components
- [ ] **Test complete pipeline**
  - Build game code
  - Pack assets
  - Build engine
  - Run engine and verify game loads
  - Test asset access
- [ ] **Document the system**
  - Create developer guide
  - Add build instructions
  - Document API

---

## Conclusion

This build system separates concerns cleanly:

- **Game code** is isolated in a DLL (easy to update/patch)
- **Assets** are packed efficiently (fast load, easy distribution)
- **Engine** is the stable runtime (stays the same across updates)

This allows you to update just the Game.dll or Assetpack.bin without rebuilding the entire engine, similar to how the example engine works.
