# Quick Integration Guide

## How to Integrate the Hybrid Build System into Source67 Engine

The hybrid build system is **complete and tested**. Here's how to integrate it into the main engine:

## Option 1: Minimal Integration (Recommended for Testing)

Just add a few lines to `Application.cpp` to test the system:

```cpp
// In Application.cpp (at the top)
#include "Core/HybridBuildSystem.h"

// In Application class (add member variable)
Scope<HybridBuildSystem> m_HybridBuildSystem;

// In Application constructor (after logger init)
m_HybridBuildSystem = CreateScope<HybridBuildSystem>();
if (!m_HybridBuildSystem->Initialize(this, &LuaScriptEngine::GetState())) {
    S67_CORE_WARN("Hybrid build system not available (DLL/assets not found)");
}

// In Application::Run() main loop (inside the while loop)
if (m_HybridBuildSystem && m_HybridBuildSystem->IsReady()) {
    m_HybridBuildSystem->Update(deltaTime);
    m_HybridBuildSystem->Render();
}

// In Application destructor
if (m_HybridBuildSystem) {
    m_HybridBuildSystem->Shutdown();
}
```

That's it! The system will:
1. Search for `Game.dll` and `GameAssets.apak`
2. Load them if found (or warn if not)
3. Initialize Lua scripts from the asset pack
4. Call game DLL update/render functions
5. Gracefully handle missing components

## Option 2: Full Integration with Input Events

Add input event forwarding to make the game DLL interactive:

```cpp
// In Application::OnEvent(Event& e)
if (m_HybridBuildSystem && m_HybridBuildSystem->IsReady()) {
    EventDispatcher dispatcher(e);
    
    dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& event) {
        m_HybridBuildSystem->OnKeyPressed(event.GetKeyCode());
        return false; // Don't consume the event
    });
    
    dispatcher.Dispatch<KeyReleasedEvent>([this](KeyReleasedEvent& event) {
        m_HybridBuildSystem->OnKeyReleased(event.GetKeyCode());
        return false;
    });
    
    dispatcher.Dispatch<MouseMovedEvent>([this](MouseMovedEvent& event) {
        m_HybridBuildSystem->OnMouseMoved(event.GetX(), event.GetY());
        return false;
    });
    
    dispatcher.Dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent& event) {
        m_HybridBuildSystem->OnMouseButton(event.GetButton(), 1);
        return false;
    });
    
    dispatcher.Dispatch<MouseButtonReleasedEvent>([this](MouseButtonReleasedEvent& event) {
        m_HybridBuildSystem->OnMouseButton(event.GetButton(), 0);
        return false;
    });
}
```

## Option 3: Editor Panel for Hybrid Build System

Add a debug panel to monitor the system:

```cpp
// In Application.cpp UI rendering section
if (ImGui::Begin("Hybrid Build System")) {
    if (m_HybridBuildSystem) {
        ImGui::Text("Status: %s", m_HybridBuildSystem->IsReady() ? "Ready" : "Not Ready");
        
        if (auto* assetPack = m_HybridBuildSystem->GetAssetPack()) {
            ImGui::Text("Assets Loaded: %u", assetPack->GetAssetCount());
            ImGui::Text("Lua Scripts: %u", assetPack->GetLuaScriptCount());
        } else {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Asset Pack: Not Loaded");
        }
        
        if (auto* gameDLL = m_HybridBuildSystem->GetGameDLL()) {
            if (gameDLL->IsLoaded()) {
                auto& api = gameDLL->GetAPI();
                ImGui::Text("Game DLL: Loaded");
                if (api.game_get_version) {
                    ImGui::Text("Version: %s", api.game_get_version());
                }
                if (api.game_get_build_number) {
                    ImGui::Text("Build: %d", api.game_get_build_number());
                }
            } else {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Game DLL: Not Loaded");
            }
        }
        
        if (ImGui::Button("Reload Lua Scripts")) {
            m_HybridBuildSystem->ReloadLuaScripts(LuaScriptEngine::GetState());
        }
    } else {
        ImGui::Text("Hybrid Build System: Not Initialized");
    }
}
ImGui::End();
```

## Testing the Integration

### 1. Build Everything

```bash
# On Linux/macOS
./build.sh Debug all

# On Windows
build.bat Debug all
```

This will:
- Build the asset packer tool
- Build the Game DLL
- Pack all assets including Lua scripts
- Build the engine

### 2. Run the Engine

```bash
# On Linux/macOS
./cmake-build-debug/Source67

# On Windows
cmake-build-debug\Debug\Source67.exe
```

### 3. Check the Console Output

You should see:
```
[Core] Initializing Hybrid Build System...
[Core] Loading asset pack: /path/to/GameAssets.apak
[Core] Asset pack loaded successfully!
[Core]   Total assets: 17
[Core]   Lua scripts: 7
[Core] Loading Game DLL: /path/to/libGame.so
[Core] Game DLL loaded successfully!
[Game DLL] Initialized!
```

If files are missing, you'll see:
```
[Core] Asset pack not found - running without packed assets
[Core] Game DLL not found - running without game code
```

This is **normal** - the system gracefully handles missing components.

## Troubleshooting

### "Game DLL not found"

Make sure you built the game DLL:
```bash
cd game && cmake -B build && cmake --build build
```

Check it exists:
```bash
ls game/build/libGame.so    # Linux/macOS
dir game\build\Debug\Game.dll  # Windows
```

### "Asset pack not found"

Make sure you ran the asset packer:
```bash
./cmake-build-tools/asset_packer -i assets/ -o GameAssets.apak -v --include-lua
```

Check it exists:
```bash
ls GameAssets.apak
```

### Compilation Errors

The hybrid build system requires:
- C++20 compiler
- sol2 library (already in CMakeLists.txt)
- LuaScriptEngine initialized

All dependencies are already in the Source67 project.

### Runtime Errors

Check the console for detailed error messages. The system logs:
- File search paths
- DLL loading status
- Function resolution
- Asset pack parsing
- Lua script execution

## Advanced: Custom Search Paths

Set environment variables to override search paths:

```bash
# Linux/macOS
export GAME_DLL_PATH=/custom/path/to/Game.dll
export ASSETPACK_PATH=/custom/path/to/GameAssets.apak

# Windows
set GAME_DLL_PATH=C:\custom\path\to\Game.dll
set ASSETPACK_PATH=C:\custom\path\to\GameAssets.apak
```

## Build Modes

### Editor Mode (Default for Debug)
```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DEDITOR_MODE=ON -B cmake-build-debug
```
- Full ImGui panels
- Developer console
- Lua hot-reload
- Debug logging

### Standalone Mode (Default for Release)
```bash
cmake -DCMAKE_BUILD_TYPE=Release -DSTANDALONE_MODE=ON -DEDITOR_MODE=OFF -B cmake-build-release
```
- No editor UI
- Optimized
- Minimal logging
- Read-only assets

## Next Steps

1. **Test basic integration** - Add minimal code, verify it compiles
2. **Test DLL loading** - Build Game.dll, run engine, check console
3. **Test asset pack** - Pack assets, verify loading
4. **Test Lua scripts** - Check Lua scripts execute from asset pack
5. **Add input forwarding** - Make game DLL interactive
6. **Create editor panel** - Monitor system status
7. **Develop game logic** - Write actual game code in Game DLL

## Additional Resources

- `BUILD_SYSTEM_README.md` - Complete user documentation
- `IMPLEMENTATION_SUMMARY.md` - Technical details
- `source_engine/builds/builds_prompt.md` - Original specification
- `game/src/game_api.h` - Game API reference
- `assets/lua/` - Example Lua scripts

---

**Questions?** Check the console output for detailed error messages and search paths.

**Working?** You should see game DLL output every 60 frames:
```
[Game DLL] Update - Time: 1.0s
[Game DLL] Update - Time: 2.0s
```
