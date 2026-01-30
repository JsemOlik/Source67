# Source67 Hybrid Build System

## Overview

The Source67 engine uses a **hybrid build system** that separates game code and assets from the engine runtime:

- **Game.dll** - C++ game code compiled to dynamic library
- **GameAssets.apak** - Binary asset pack (scenes, models, textures, shaders, **Lua scripts**)  
- **Source67.exe** - Engine runtime that loads both DLL and asset pack

## Quick Start

### Linux/macOS

```bash
# Build everything (engine + game + assets)
./build.sh Release all

# Build specific components
./build.sh Debug game      # Build Game.dll only
./build.sh Debug tools     # Build asset packer only
./build.sh Debug assets    # Pack assets only
./build.sh Debug engine    # Build engine only

# Run
./cmake-build-release/Source67
```

### Windows

```cmd
REM Build everything (engine + game + assets)
build.bat Release all

REM Build specific components
build.bat Debug game       REM Build Game.dll only
build.bat Debug tools      REM Build asset packer only
build.bat Debug assets     REM Pack assets only
build.bat Debug engine     REM Build engine only

REM Run
cmake-build-release\Source67.exe
```

## Build Modes

### Editor Mode (Development)

```bash
./build.sh Debug
```

Features:
- Full ImGui editor panels
- Scene hierarchy, inspector, content browser
- Developer console
- Gizmo manipulation
- Lua hot-reload support
- Debug logging

### Standalone Mode (Shipping)

```bash
./build.sh Release
```

Features:
- No editor UI
- Optimized performance
- Minimal logging
- Read-only Lua scripts from asset pack

## Project Structure

```
Source67/
├── src/                      # Engine source code
│   └── Core/
│       ├── HybridBuildSystem.h/cpp  # DLL + Asset pack runtime
├── game/                     # Game DLL source
│   ├── src/
│   │   ├── game_api.h       # C API exported to engine
│   │   ├── game_api.cpp     # API implementation
│   │   └── Components/      # Game components
│   └── CMakeLists.txt       # Game DLL build config
├── tools/
│   └── asset_packer/        # Asset packer tool
│       ├── AssetPacker.h/cpp
│       └── main.cpp
├── assets/
│   ├── scenes/              # Scene files (.s67)
│   ├── models/              # 3D models
│   ├── textures/            # Texture files
│   ├── shaders/             # GLSL shaders
│   ├── fonts/               # Font files
│   └── lua/                 # Lua game scripts ⭐
│       ├── gameplay/        # Gameplay logic
│       │   ├── player_controller.lua
│       │   ├── enemy_ai.lua
│       │   └── game_manager.lua
│       ├── ui/              # UI scripts
│       │   ├── hud.lua
│       │   └── menu.lua
│       └── util/            # Utility scripts
│           ├── math.lua
│           └── helpers.lua
├── build.sh / build.bat     # Build scripts
└── CMakeLists.txt           # Root build config
```

## Asset Packer

The asset packer creates a binary `.apak` file containing all game assets:

```bash
# Manual packing
./cmake-build-tools/asset_packer -i assets/ -o GameAssets.apak -v --include-lua

# Options
-i, --input <dir>      Input assets directory
-o, --output <file>    Output .apak file
-v, --verbose          Verbose output
--include-lua          Include Lua scripts (default: yes)
--lua-dir <dir>        Lua scripts subdirectory (default: lua)
--validate             Validate pack after creation
```

## Game DLL Development

### Creating Game Components

```cpp
// game/src/Components/PlayerComponent.h
class PlayerComponent {
public:
    void OnCreate();
    void OnUpdate(float deltaTime);
    void OnDestroy();
private:
    float m_Speed = 5.0f;
};
```

### Game API Functions

The Game DLL exports these functions (see `game/src/game_api.h`):

```cpp
extern "C" {
    void game_initialize(void* engine_context, void* lua_state);
    void game_update(float delta_time);
    void game_render();
    void game_on_key_pressed(int key_code);
    void game_on_assets_loaded(void* assetpack_handle);
    // ... more callbacks
}
```

## Lua Scripting

### Example Lua Script

```lua
-- assets/lua/gameplay/player_controller.lua
local PlayerController = {}

function PlayerController:OnCreate()
    self.speed = 5.0
    self.health = 100
    print("[Lua] Player created!")
end

function PlayerController:OnUpdate(dt)
    -- Game logic here
    if self.health < 50 then
        print("[Lua] Low health!")
    end
end

return PlayerController
```

### Lua API Bindings

The engine exposes C++ functions to Lua (configured in `LuaScriptEngine.cpp`):

```lua
-- Available in Lua:
local input = GetInput()
local entity = GetEntity()
DrawText("Hello World", 10, 10)
```

## Runtime Behavior

### Startup Sequence

1. Engine initializes (OpenGL, ImGui, Physics)
2. Search for `Game.dll` and `GameAssets.apak`
3. Load asset pack into memory
4. Load Lua scripts from asset pack
5. Load Game DLL dynamically
6. Resolve game API functions
7. Call `game_initialize()` with engine context and Lua state
8. Call `game_on_assets_loaded()`
9. Enter main loop

### Search Paths

**Game DLL:**
- `./Game.dll`
- `./game/build/Release/Game.dll`
- `./game/build/Debug/Game.dll`
- `$GAME_DLL_PATH` environment variable

**Asset Pack:**
- `./GameAssets.apak`
- `./assets/GameAssets.apak`
- `./build/GameAssets.apak`
- `$ASSETPACK_PATH` environment variable

## Distribution

### Standalone Release Package

```
Source67_Release/
├── Source67.exe          # Engine runtime
├── GameAssets.apak       # Packed assets + Lua scripts
└── Game.dll              # Game code
```

All three files are required. Missing any component shows a clear error message.

### Editor/Development Package

Same as release, but includes:
- Debug symbols
- Editor panels
- Lua hot-reload
- Developer console

## Build System Options (CMake)

```cmake
# Build options
option(STANDALONE_MODE "Build without editor features" OFF)
option(EDITOR_MODE "Build with full debug features" ON)
option(BUILD_GAME_DLL "Build Game.dll" ON)
option(BUILD_ASSET_PACKER "Build asset packer tool" ON)
```

### Custom CMake Targets

```bash
# Build only the asset packer
cmake --build build --target asset_packer

# Pack assets
cmake --build build --target pack_assets

# Complete build (engine + game + assets)
cmake --build build --target build_complete
```

## Troubleshooting

### "Game DLL not found"

Check search paths and ensure Game.dll was built:
```bash
ls game/build/Release/Game.dll
```

Set custom path:
```bash
export GAME_DLL_PATH=/path/to/Game.dll
```

### "Asset pack not found"

Ensure asset packing completed:
```bash
ls GameAssets.apak
```

Set custom path:
```bash
export ASSETPACK_PATH=/path/to/GameAssets.apak
```

### Lua script errors

Check logs for detailed error messages. Lua syntax errors are caught and logged without crashing the engine.

### DLL API resolution failed

Ensure all required functions are exported from Game.dll. Check that `game_api.cpp` implements all functions defined in `game_api.h`.

## Hot-Reload (Editor Mode Only)

### Lua Scripts

Lua scripts can be reloaded without restarting:
- Edit script in `assets/lua/`
- Re-pack assets
- Press "Reload Lua Scripts" in editor

### Game DLL

Hot-reloading Game.dll requires:
1. Build new DLL
2. Unload current DLL
3. Reload new DLL
4. Re-initialize game state

(Not yet fully implemented - requires state serialization)

## Advanced: Dual-Scripting

Combine C++ performance with Lua flexibility:

**C++ Component:**
```cpp
class PlayerComponent : public ScriptableEntity {
    void OnUpdate(float ts) override {
        // Call Lua script
        m_LuaController["OnUpdate"](ts);
    }
private:
    sol::table m_LuaController;
};
```

**Lua Script:**
```lua
function OnUpdate(dt)
    -- Lua gameplay logic
end
```

## Performance Notes

- Asset pack loads entirely into memory on startup
- Lua scripts compiled once at load time
- Game DLL calls have minimal overhead (direct function pointers)
- Asset pack uses FNV-1a hashing for fast lookups

## Next Steps

1. Implement asset compression (LZ4)
2. Add asset streaming for large files
3. Implement Game DLL hot-reload with state preservation
4. Add encrypted asset packs for shipping
5. Implement async asset loading

---

For more information, see:
- `/source_engine/builds/builds_prompt.md` - Full specification
- `/game/src/game_api.h` - Game API reference
- `/assets/lua/` - Example Lua scripts
