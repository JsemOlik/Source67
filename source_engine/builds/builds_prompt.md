# Source67 Engine - Hybrid Build System Specification

## Overview

This document specifies a complete build system for **Source67**, a modern C++20 3D game engine with OpenGL 4.5+, Jolt Physics v5.0.0, ImGui (docking branch), and **dual-scripting support** (C++ native + Lua via sol2). The system separates game code and assets from the engine runtime using a DLL-based architecture with custom asset packing.

**Architecture Pattern (Source67 Hybrid):**

- **C++ Game Code** compiles to a single `Game.dll` dynamic library (native game logic, custom components, entity behavior)
- **Lua Game Scripts** package into the `GameAssets.apak` file (hot-reloadable gameplay scripts, entity behaviors, game logic)
- **Assets** pack into `GameAssets.apak` binary file (scenes `.s67`, models, textures, shaders, fonts, **Lua scripts**)
- **Engine** builds into standalone executable `Source67.exe` that loads both DLL and asset pack at runtime
- **Editor Mode** includes ImGui panels, gizmos, debugging, asset browser, developer console (stripped in standalone)
- **Full Integration** with Jolt Physics, spdlog logging, sol2 Lua bindings, existing renderer pipeline

---

## Build Pipeline Overview

```
┌──────────────────────────────────────────────────────────────────────┐
│                     SOURCE67 BUILD PROCESS                           │
└──────────────────────────────────────────────────────────────────────┘

[C++ Game Code]  [Lua Scripts]  [Asset Files]   [Engine Source Code]
        │              │              │                    │
        │              │              │                    │
        ▼              ▼              ▼                    ▼
    ┌────────────┐ ┌────────────┐ ┌──────────┐       ┌─────────────┐
    │C++20       │ │Lua Scripts │ │Asset     │       │C++20 Compiler
    │ Compiler   │ │(packaged)  │ │Packer    │       │ + GLAD+    │
    │ (MSVC)     │ │into apak   │ │Tool      │       │ImGui+Jolt  │
    └────────────┘ └────────────┘ └──────────┘       └─────────────┘
        │              │              │                    │
        ▼              ▼              ▼                    ▼
   [Game.dll]    ┌─────────────────────────┐          [Source67.exe]
                 │   GameAssets.apak       │
                 ├─────────────────────────┤
                 │ - Scenes (.s67)         │
                 │ - Models/Textures       │
                 │ - Shaders (.glsl)       │
                 │ - Fonts                 │
                 │ - Lua Scripts (.lua)    │
                 │ - Metadata              │
                 └─────────────────────────┘
                         │
        ┌────────────────┼────────────────┐
        │                │                │
        ▼                ▼                ▼
    [Game.dll]  [GameAssets.apak]  [Source67.exe]
        │                │                │
        └────────────────┼────────────────┘
                         │
                         ▼
            ┌──────────────────────────┐
            │   Runtime Startup        │
            │   (Source67.exe main())  │
            └──────────────────────────┘
                         │
        ┌────────────────┼────────────────┐
        │                │                │
        ▼                ▼                ▼
    Load DLL    Load Asset Pack    Initialize
    (Game.dll)  (GameAssets.apak)  OpenGL/Physics/
                                    ImGui/Lua
                         │
                         ▼
            ┌──────────────────────────┐
            │   Running Game           │
            │ (Editor or Standalone)   │
            └──────────────────────────┘
```

---

## Part 1: C++ Game Code Compilation (Game.dll)

### 1.1 Game Source Structure

```
game/
├── src/
│   ├── game_dll_main.cpp         # DLL entry point
│   ├── game_api.h                # API exposed to engine (C-compatible)
│   ├── game_api.cpp              # API implementation
│   ├── Components/
│   │   ├── PlayerComponent.h
│   │   ├── PlayerComponent.cpp
│   │   ├── EnemyComponent.h
│   │   └── EnemyComponent.cpp
│   ├── Systems/
│   │   ├── GameLogicSystem.h
│   │   └── GameLogicSystem.cpp
│   ├── Scripts/
│   │   ├── MyGameScript.h
│   │   └── MyGameScript.cpp
│   └── CMakeLists.txt
├── CMakeLists.txt                # Game build config
└── build/
    └── Release/
        └── Game.dll              # Compiled output
```

### 1.2 Game API Interface (C-Compatible)

The game DLL exposes a C-compatible interface that the engine calls:

```cpp
// game_api.h - Exported API for engine to call

#ifdef _WIN32
    #define GAME_API __declspec(dllexport)
#else
    #define GAME_API __attribute__((visibility("default")))
#endif

extern "C" {
    // Initialization with engine context and Lua state
    void game_initialize(void* engine_context, void* lua_state);

    // Core game loop
    void game_update(float delta_time);
    void game_render();

    // Input callbacks
    void game_on_key_pressed(int key_code);
    void game_on_key_released(int key_code);
    void game_on_mouse_moved(float x, float y);
    void game_on_mouse_button(int button, int action);

    // Asset and scene loading
    void game_on_assets_loaded(void* assetpack_handle);
    void game_on_scene_loaded(const char* scene_path);

    // Lua integration
    void game_on_lua_script_loaded(const char* script_path);
    void game_on_lua_script_reloaded(const char* script_path);

    // Lifecycle
    void game_shutdown();

    // Metadata
    const char* game_get_version();
    int game_get_build_number();
}
```

### 1.3 Game API Implementation Pattern

```cpp
// game_api.cpp
#include "game_api.h"
#include "Source67/S67.h"

// Global state
static S67::Ref<S67::Scene> g_scene;
static void* g_assetpack = nullptr;
static sol::state* g_lua = nullptr;

GAME_API void game_initialize(void* engine_context, void* lua_state) {
    // Store lua state for script access
    g_lua = static_cast<sol::state*>(lua_state);

    // Create game scene
    g_scene = S67::CreateRef<S67::Scene>();

    S67_INFO("Game DLL Initialized!");
}

GAME_API void game_update(float delta_time) {
    // Update game logic
    if (g_scene) {
        g_scene->OnUpdate(delta_time);
    }
}

GAME_API void game_render() {
    // Render game (engine handles most rendering)
    // Game can queue custom render calls
}

GAME_API void game_on_assets_loaded(void* assetpack_handle) {
    g_assetpack = assetpack_handle;
    // Load initial scene
    game_on_scene_loaded("scenes/main.s67");
}

GAME_API void game_on_lua_script_loaded(const char* script_path) {
    // Called when Lua script is loaded from assets
    if (g_lua) {
        try {
            g_lua->script_file(script_path);
            S67_INFO("Lua script loaded: {}", script_path);
        } catch (const sol::error& e) {
            S67_ERROR("Failed to load Lua script {}: {}", script_path, e.what());
        }
    }
}

// ... other API functions ...
```

### 1.4 Native C++ Script Component Pattern

Game can use Source67's `ScriptableEntity` for native C++ components:

```cpp
// game/src/Components/PlayerComponent.h
#pragma once
#include "Source67/Renderer/ScriptableEntity.h"

class PlayerComponent : public S67::ScriptableEntity {
public:
    void OnCreate() override;
    void OnUpdate(float ts) override;
    void OnEvent(S67::Event& e) override;
    void OnDestroy() override;

private:
    float speed = 5.0f;
};

// Registration (placed in game_api.cpp or ComponentRegistry)
#include "Source67/Renderer/ScriptRegistry.h"
REGISTER_SCRIPT(PlayerComponent);
```

### 1.5 Compilation Process

**Build with CMake:**

```bash
cd game/
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build --config Release

# Output: game/build/Release/Game.dll
# On Linux: game/build/Release/libGame.so
# On macOS: game/build/Release/libGame.dylib
```

**Game CMakeLists.txt:**

```cmake
cmake_minimum_required(VERSION 3.20)
project(GameDLL)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include Source67 headers (from engine installation or relative path)
include_directories(${CMAKE_SOURCE_DIR}/../src)

# Add source files
file(GLOB_RECURSE GAME_SOURCES "src/**/*.cpp")

# Create DLL
add_library(Game SHARED ${GAME_SOURCES})

# Link to Source67 (or just headers if static linking engine)
target_include_directories(Game PRIVATE ${CMAKE_SOURCE_DIR}/../src)

# Output to Release/Debug subdirectory
set_target_properties(Game PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)
```

---

## Part 2: Lua Scripts & Asset Packing (GameAssets.apak)

### 2.1 Asset File Structure

```
assets/
├── scenes/
│   ├── main.s67                  # Main scene (JSON format)
│   ├── level_01.s67
│   └── level_02.s67
├── models/
│   ├── player.obj
│   ├── enemy.fbx
│   └── building.obj
├── textures/
│   ├── player_diffuse.png
│   ├── terrain.png
│   └── ui_atlas.png
├── shaders/
│   ├── lighting.glsl
│   ├── texture.glsl
│   └── custom_shader.glsl
├── fonts/
│   ├── Roboto-Medium.ttf
│   └── monospace.ttf
├── lua/                          # NEW: Lua game scripts
│   ├── gameplay/
│   │   ├── player_controller.lua
│   │   ├── enemy_ai.lua
│   │   └── game_manager.lua
│   ├── ui/
│   │   ├── hud.lua
│   │   └── menu.lua
│   └── util/
│       ├── math.lua
│       └── helpers.lua
└── config.json                   # Asset metadata
```

### 2.2 Lua Script Examples

**Gameplay Script:**

```lua
-- assets/lua/gameplay/player_controller.lua

local PlayerController = {}

function PlayerController:OnCreate()
    self.speed = 5.0
    self.health = 100
    print("Player created with speed:", self.speed)
end

function PlayerController:OnUpdate(dt)
    -- Check input from engine
    local input = GetInput()

    if input.forward then
        local transform = self:GetComponent("Transform")
        transform.position.y = transform.position.y + self.speed * dt
    end
end

function PlayerController:TakeDamage(amount)
    self.health = self.health - amount
    print("Player health:", self.health)

    if self.health <= 0 then
        self:Die()
    end
end

function PlayerController:Die()
    print("Player died!")
    self:Destroy()
end

return PlayerController
```

**UI Script:**

```lua
-- assets/lua/ui/hud.lua

local HUD = {}

function HUD:OnCreate()
    self.visible = true
end

function HUD:OnUpdate(dt)
    -- Render HUD elements (through engine's HUDRenderer)
    if self.visible then
        DrawText("Health: " .. GetPlayerHealth(), 10, 10)
        DrawText("Score: " .. GetScore(), 10, 30)
    end
end

function HUD:SetVisible(visible)
    self.visible = visible
end

return HUD
```

### 2.3 Asset Packer Implementation

The asset packer scans assets/ and creates a binary package including Lua scripts:

**Enhanced Assetpack Binary Format:**

```
[HEADER]
├── Magic: "AP67" (4 bytes)       # Changed from "APAK" for Source67
├── Version: 2 (4 bytes)
├── Asset Count: N (4 bytes)
├── Index Offset: (8 bytes)
├── Lua Script Count: M (4 bytes)
├── Flags: [has_compression, has_encryption] (4 bytes)
├── Reserved: (8 bytes)
│
[ASSET DATA SECTION]
├── Scene 1 (.s67 JSON)
├── Model 1 (binary)
├── Texture 1 (PNG/etc)
├── Shader 1 (GLSL text)
├── Lua Script 1 (text)
├── Lua Script 2 (text)
├── ... N total assets ...
│
[INDEX TABLE]
├── Entry 1: { path_hash, type, offset, size, compression }
├── Entry 2: ...
├── Entry M: ... (Lua script entries)
│
[LUA SCRIPT INDEX] (NEW)
├── Script 1: { path, hash, offset, size }
├── Script 2: ...
├── ... M script entries ...
│
[FOOTER]
├── Checksum (8 bytes)
└── Metadata Checksum (8 bytes)
```

### 2.4 Asset Type Enumeration (Extended)

```cpp
enum AssetType : uint32_t {
    // Existing types
    ASSET_UNKNOWN = 0,
    ASSET_TEXTURE = 1,
    ASSET_MODEL = 2,
    ASSET_SCENE = 3,
    ASSET_SHADER = 4,
    ASSET_FONT = 5,

    // NEW: Lua support
    ASSET_LUA_SCRIPT = 6,
    ASSET_CONFIG_JSON = 7,

    // Future
    ASSET_AUDIO = 8,
    ASSET_ANIMATION = 9,
};
```

### 2.5 Asset Packer Tool

```bash
# Command-line interface
./asset_packer -i assets/ -o GameAssets.apak -c lz4 -v --validate

# Options:
# -i, --input <dir>           Input assets directory
# -o, --output <file>         Output Assetpack filename
# -c, --compression <type>    Compression (none, deflate, lz4)
# -v, --verbose               Verbose output
# --validate                  Validate integrity
# --include-lua               Include Lua scripts (default: yes)
# --lua-dir <dir>             Lua scripts subdirectory (default: lua/)
```

### 2.6 Packer Implementation Snippet

```cpp
// tools/asset_packer/AssetPacker.cpp

struct AssetPackerContext {
    std::vector<AssetEntry> entries;
    std::vector<LuaScriptEntry> lua_scripts;
    std::ofstream output_file;
    uint64_t current_offset = 0;
};

void PackAssets(const std::string& input_dir, const std::string& output_file) {
    AssetPackerContext ctx;

    // Scan regular assets
    for (const auto& entry : fs::recursive_directory_iterator(input_dir)) {
        if (entry.is_regular_file()) {
            AssetType type = DetermineAssetType(entry.path());
            ctx.entries.push_back(PackAsset(entry.path(), type, ctx.current_offset));
            ctx.current_offset += GetFileSize(entry.path());
        }
    }

    // Scan Lua scripts (NEW)
    auto lua_dir = fs::path(input_dir) / "lua";
    if (fs::exists(lua_dir)) {
        for (const auto& entry : fs::recursive_directory_iterator(lua_dir)) {
            if (entry.path().extension() == ".lua") {
                ctx.lua_scripts.push_back(PackLuaScript(entry.path(), ctx.current_offset));
                ctx.current_offset += GetFileSize(entry.path());
            }
        }
    }

    // Write to file
    WriteAssetPackHeader(ctx, output_file);
    WriteAssetData(ctx, output_file);
    WriteIndexTable(ctx, output_file);
    WriteLuaScriptIndex(ctx, output_file);
    WriteFooter(ctx, output_file);
}
```

---

## Part 3: Engine Runtime (Source67.exe)

### 3.1 Enhanced Engine Startup Sequence

When `Source67.exe` launches in **Standalone Mode**:

```cpp
// src/Core/Application.cpp - Standalone initialization

int main(int argc, char* argv[]) {
    try {
        // Step 1: Initialize GLFW and window
        S67::Application app("Source67 - Standalone");

        // Step 2: Initialize OpenGL (GLAD)
        app.Initialize();

        // Step 3: Initialize Lua runtime
        sol::state lua;
        lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table);

        // Step 4: Search for Game.dll and GameAssets.apak
        std::string game_dll_path = app.FindGameDLL();
        std::string assetpack_path = app.FindAssetPack();

        if (game_dll_path.empty() || assetpack_path.empty()) {
            throw std::runtime_error("Game DLL or asset pack not found!");
        }

        // Step 5: Load asset pack into memory
        void* assetpack_handle = app.LoadAssetPack(assetpack_path);

        // Step 6: Load Lua scripts from asset pack (NEW)
        app.LoadLuaScriptsFromAssets(assetpack_handle, lua);

        // Step 7: Load Game.dll dynamically
        void* game_dll_handle = app.LoadGameDLL(game_dll_path);

        // Step 8: Resolve game API functions
        GameAPI game_api = app.ResolveGameAPI(game_dll_handle);

        // Step 9: Initialize game with both engine context and Lua state
        game_api.game_initialize(&app, &lua);

        // Step 10: Notify game that assets are loaded
        game_api.game_on_assets_loaded(assetpack_handle);

        // Step 11: Main loop
        while (!glfwWindowShouldClose((GLFWwindow*)app.GetWindowHandle())) {
            float delta_time = app.GetDeltaTime();

            // Physics update
            app.UpdatePhysics(delta_time);

            // Game update
            game_api.game_update(delta_time);

            // Render
            game_api.game_render();
            app.RenderFrame();

            glfwSwapBuffers((GLFWwindow*)app.GetWindowHandle());
            glfwPollEvents();
        }

        // Step 12: Shutdown
        game_api.game_shutdown();
        app.UnloadGameDLL(game_dll_handle);
        app.UnloadAssetPack(assetpack_handle);
        app.Shutdown();

        return 0;
    }
    catch (const std::exception& e) {
        S67_CORE_ERROR("Engine error: {}", e.what());
        return 1;
    }
}
```

### 3.2 Finding Game Components

```cpp
std::string Application::FindGameDLL() {
    std::vector<std::string> search_paths = {
        "./Game.dll",
        "./game/build/Release/Game.dll",
        "../game/build/Release/Game.dll",
        std::getenv("GAME_DLL_PATH") ? std::getenv("GAME_DLL_PATH") : "",
    };

    for (const auto& path : search_paths) {
        if (std::filesystem::exists(path)) {
            return std::filesystem::absolute(path).string();
        }
    }
    return "";
}

std::string Application::FindAssetPack() {
    std::vector<std::string> search_paths = {
        "./GameAssets.apak",
        "./assets/GameAssets.apak",
        "../assets/GameAssets.apak",
        std::getenv("ASSETPACK_PATH") ? std::getenv("ASSETPACK_PATH") : "",
    };

    for (const auto& path : search_paths) {
        if (std::filesystem::exists(path)) {
            return std::filesystem::absolute(path).string();
        }
    }
    return "";
}
```

### 3.3 Lua Script Loading from Assets (NEW)

```cpp
// src/Scripting/LuaScriptEngine.cpp

void Application::LoadLuaScriptsFromAssets(void* assetpack_handle, sol::state& lua) {
    AssetpackData* pack = (AssetpackData*)assetpack_handle;

    // Get Lua script index from asset pack
    auto lua_scripts = pack->GetLuaScriptIndex();

    for (const auto& script_entry : lua_scripts) {
        try {
            // Get script data from asset pack
            const uint8_t* script_data = pack->GetAssetData(script_entry.path);
            std::string script_code(
                (const char*)script_data,
                script_entry.size
            );

            // Execute in Lua state
            lua.script(script_code, script_entry.path);

            S67_CORE_INFO("Loaded Lua script: {}", script_entry.path);
        }
        catch (const sol::error& e) {
            S67_CORE_ERROR("Failed to load Lua script {}: {}", script_entry.path, e.what());
        }
    }
}
```

### 3.4 Dynamic DLL Loading (Unchanged)

```cpp
void* Application::LoadGameDLL(const std::string& dll_path) {
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
```

### 3.5 API Function Resolution

```cpp
struct GameAPI {
    void (*game_initialize)(void* engine_context, void* lua_state);
    void (*game_update)(float delta_time);
    void (*game_render)();
    void (*game_on_key_pressed)(int key_code);
    void (*game_on_key_released)(int key_code);
    void (*game_on_mouse_moved)(float x, float y);
    void (*game_on_mouse_button)(int button, int action);
    void (*game_on_assets_loaded)(void* assetpack_handle);
    void (*game_on_scene_loaded)(const char* scene_path);
    void (*game_on_lua_script_loaded)(const char* script_path);
    void (*game_on_lua_script_reloaded)(const char* script_path);
    void (*game_shutdown)();
};

GameAPI Application::ResolveGameAPI(void* dll_handle) {
    GameAPI api = {};

    auto resolve = [dll_handle](const char* name) -> void* {
        #ifdef _WIN32
            return (void*)GetProcAddress((HMODULE)dll_handle, name);
        #else
            return dlsym(dll_handle, name);
        #endif
    };

    api.game_initialize = (decltype(api.game_initialize))resolve("game_initialize");
    api.game_update = (decltype(api.game_update))resolve("game_update");
    api.game_render = (decltype(api.game_render))resolve("game_render");
    // ... resolve others ...

    if (!api.game_initialize || !api.game_update || !api.game_render) {
        throw std::runtime_error("Failed to resolve required game API functions");
    }

    return api;
}
```

---

## Part 4: Build Modes

### 4.1 Editor Mode

When running in **Editor Mode** (development):

```cpp
#ifdef EDITOR_MODE
    // Keep all ImGui panels visible
    engine.show_scene_hierarchy_panel();
    engine.show_inspector_panel();
    engine.show_content_browser_panel();
    engine.show_viewport_panel();
    engine.show_developer_console();

    // Enable Lua hot-reload
    engine.enable_lua_hot_reload();

    // Enable gizmos
    engine.enable_gizmos();

    // Detailed logging
    spdlog::set_level(spdlog::level::debug);
#endif
```

**Build for Editor:**

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DEDITOR_MODE=ON -B cmake-build-debug
cmake --build cmake-build-debug
```

### 4.2 Standalone Mode

When running in **Standalone Mode** (shipping):

```cpp
#ifdef STANDALONE_MODE
    // Hide all editor UI
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

    // Disable Lua hot-reload (scripts are readonly)
    engine.disable_lua_hot_reload();

    // Disable gizmos
    engine.disable_gizmos();

    // Minimal logging
    spdlog::set_level(spdlog::level::err);
#endif
```

**Build for Standalone:**

```bash
cmake -DCMAKE_BUILD_TYPE=Release -DSTANDALONE_MODE=ON -B cmake-build-release
cmake --build cmake-build-release
```

---

## Part 5: Complete Build Script

### 5.1 Master Build Script (`build.sh`)

```bash
#!/bin/bash
# Complete build for Source67 + Game.dll + GameAssets.apak

set -e

BUILD_TYPE=${1:-Release}
TARGET=${2:-all}

echo "========================================="
echo "Source67 Engine - Hybrid Build System"
echo "========================================="
echo "Build Type: $BUILD_TYPE"
echo "Target: $TARGET"
echo ""

# Step 1: Build Game.dll
if [ "$TARGET" = "all" ] || [ "$TARGET" = "game" ]; then
    echo "[1/3] Building Game.dll..."
    cd game
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -B build
    cmake --build build --config $BUILD_TYPE
    echo "✓ Game.dll compiled"
    cd ..
    echo ""
fi

# Step 2: Pack Assets (including Lua scripts)
if [ "$TARGET" = "all" ] || [ "$TARGET" = "assets" ]; then
    echo "[2/3] Packing assets (GameAssets.apak)..."
    ./asset_packer -i assets/ -o GameAssets.apak -c lz4 -v --validate --include-lua
    echo "✓ GameAssets.apak created"
    echo ""
fi

# Step 3: Build Engine
if [ "$TARGET" = "all" ] || [ "$TARGET" = "engine" ]; then
    echo "[3/3] Building Source67.exe..."
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DSTANDALONE_MODE=ON -B cmake-build-$BUILD_TYPE
    cmake --build cmake-build-$BUILD_TYPE --config $BUILD_TYPE
    echo "✓ Source67.exe built"
    echo ""
fi

echo "========================================="
echo "Build Complete!"
echo "========================================="
echo ""
echo "Distribution package contents:"
echo "  - cmake-build-$BUILD_TYPE/Source67.exe"
echo "  - GameAssets.apak"
echo "  - game/build/$BUILD_TYPE/Game.dll"
echo ""
echo "To run: cmake-build-$BUILD_TYPE/Source67.exe"
```

### 5.2 CMakeLists.txt Integration

Root `CMakeLists.txt` should support building as one unit or separately:

```cmake
cmake_minimum_required(VERSION 3.20)
project(Source67 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(STANDALONE_MODE "Build in standalone mode (no editor features)" OFF)
option(EDITOR_MODE "Build editor with full debug features" ON)
option(BUILD_GAME_DLL "Build Game.dll from game/ subdirectory" ON)

# Main engine
add_subdirectory(src)

# Game DLL (optional)
if(BUILD_GAME_DLL AND EXISTS "${CMAKE_SOURCE_DIR}/game/CMakeLists.txt")
    add_subdirectory(game)
endif()

# Asset packing target
add_custom_target(pack_assets
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/asset_packer
        -i ${CMAKE_CURRENT_SOURCE_DIR}/assets/
        -o ${CMAKE_BINARY_DIR}/GameAssets.apak
        -c lz4 -v --include-lua
    COMMENT "Packing GameAssets.apak..."
)

# Full build target
add_custom_target(build_complete
    DEPENDS Source67 Game pack_assets
    COMMENT "Complete build (engine + game + assets)"
)
```

---

## Part 6: File Locations and Distribution

### 6.1 Final Standalone Distribution

```
Source67_Release/
├── Source67.exe               # Main executable
├── GameAssets.apak           # Assets + Lua scripts
├── Game.dll                  # Game code (Windows)
├── Game.so / Game.dylib      # Game code (Linux/macOS)
└── README.md                 # Shipping notes
```

### 6.2 Runtime Requirements

**All three components required:**

1. **Source67.exe** - Engine runtime (contains OpenGL, Jolt, ImGui)
2. **Game.dll** - C++ game logic
3. **GameAssets.apak** - Scenes, models, textures, shaders, Lua scripts

Missing any component results in clear error:

```
ERROR: Cannot find GameAssets.apak
Searched in:
  - ./GameAssets.apak
  - ./assets/GameAssets.apak
  - $ASSETPACK_PATH environment variable
```

---

## Part 7: Lua Hot-Reload (Development Feature)

### 7.1 Editor Mode: Live Script Reloading

In **Editor Mode**, Lua scripts can be reloaded without restarting:

```cpp
// Editor: Watch assets/lua/ for changes
if (ImGui::Button("Reload Lua Scripts")) {
    app.ReloadLuaScriptsFromAssets(assetpack_handle, lua_state);
    game_api.game_on_lua_script_reloaded("*");  // Notify game of reload
}
```

### 7.2 Lua Script Hot-Reload Flow

1. Developer edits `assets/lua/gameplay/player_controller.lua`
2. Editor detects file change
3. Asset packer re-packs only changed scripts into GameAssets.apak
4. Engine reloads Lua state with new script code
5. Game receives `game_on_lua_script_reloaded()` callback
6. No engine/DLL restart needed

---

## Part 8: Developer Workflow

### 8.1 Editor/Development Workflow

```bash
# Build for editor with debug symbols and hot-reload
./build.sh Debug all

# Run editor
./cmake-build-debug/Source67

# In editor:
# - Develop scenes and entities
# - Edit Lua scripts in assets/lua/
# - Scripts hot-reload on save
# - C++ changes require rebuild
```

### 8.2 Shipping/Release Workflow

```bash
# Final build for standalone (optimized, no editor)
./build.sh Release all

# Verify distribution package
ls -la cmake-build-release/Source67.exe
ls -la GameAssets.apak
ls -la game/build/Release/Game.dll

# Package and ship
zip Source67_Release.zip \
    cmake-build-release/Source67.exe \
    GameAssets.apak \
    game/build/Release/Game.dll \
    README.md
```

---

## Part 9: Dual-Scripting Integration

### 9.1 C++ + Lua Script Coordination

**C++ Native Script (game/src/Components/PlayerComponent.h):**

```cpp
class PlayerComponent : public S67::ScriptableEntity {
    void OnCreate() override {
        // C++ initialization
        lua_state->script(R"(
            local player = GetEntity()
            player.lua_controller = require("lua/gameplay/player_controller")
            player.lua_controller:OnCreate()
        )");
    }

    void OnUpdate(float ts) override {
        // Call Lua update
        auto lua_controller = lua_state->globals()["player"]["lua_controller"];
        lua_controller["OnUpdate"](ts);
    }
};
```

**Lua Script (assets/lua/gameplay/player_controller.lua):**

```lua
local PlayerController = {speed = 5.0}

function PlayerController:OnCreate()
    print("Lua player initialized!")
end

function PlayerController:OnUpdate(dt)
    -- Lua gameplay logic
end

return PlayerController
```

### 9.2 Lua API Bindings

The engine exposes C++ functions to Lua via sol2:

```cpp
// Register engine functions for Lua
lua.set_function("GetInput", [](){ return GetCurrentInput(); });
lua.set_function("GetEntity", [](){ return GetActiveEntity(); });
lua.set_function("DrawText", [](const std::string& text, float x, float y) {
    HUDRenderer::QueueString(text, glm::vec2(x, y));
});
lua.set_function("GetPlayerHealth", [](){ return GetPlayer()->GetHealth(); });
```

---

## Part 10: Implementation Checklist

For AI agent implementing this system:

- [ ] **Extend game_api.h** with Lua callbacks
- [ ] **Implement Lua script loading** in Application class
- [ ] **Create asset packer enhancements** for .lua files
- [ ] **Update AssetType enum** to include ASSET_LUA_SCRIPT
- [ ] **Implement GameAPI struct** with all 11 function pointers
- [ ] **Create game_dll_main.cpp** entry point
- [ ] **Update CMakeLists.txt** with Lua search paths
- [ ] **Add sol2 dependency** to CMakeLists.txt (already in Source67)
- [ ] **Create example Lua scripts** in assets/lua/
- [ ] **Implement sol2 bindings** to expose engine API to Lua
- [ ] **Test build pipeline**: game → assets → engine → run
- [ ] **Test Lua hot-reload** in editor mode
- [ ] **Verify standalone mode** strips editor UI

---

## Conclusion

This **hybrid build system** for Source67 combines:

✅ **C++ Game DLL** - Performance-critical native code  
✅ **Lua Scripts** - Hot-reloadable gameplay logic  
✅ **Packed Assets** - Single efficient asset file  
✅ **Modular Engine** - Engine stays stable across updates  
✅ **Editor & Standalone** - Same binary, different modes  
✅ **Full Jolt Physics Integration** - Physics available to both C++ and Lua  
✅ **ImGui Editor UI** - Stripped in standalone builds

The system allows rapid iteration through Lua scripting while maintaining performance with C++ native code for critical systems.
