#pragma once

#include "Core/Base.h"
#include "../tools/asset_packer/AssetPackerTypes.h"
#include <filesystem>
#include <vector>
#include <unordered_map>
#include <sol/sol.hpp>

namespace S67 {

// Forward declarations
class AssetPackRuntime;

// Game API function pointers
struct GameAPI {
    void (*game_initialize)(void* engine_context, void* lua_state) = nullptr;
    void (*game_shutdown)() = nullptr;
    void (*game_update)(float delta_time) = nullptr;
    void (*game_render)() = nullptr;
    void (*game_on_key_pressed)(int key_code) = nullptr;
    void (*game_on_key_released)(int key_code) = nullptr;
    void (*game_on_mouse_moved)(float x, float y) = nullptr;
    void (*game_on_mouse_button)(int button, int action) = nullptr;
    void (*game_on_assets_loaded)(void* assetpack_handle) = nullptr;
    void (*game_on_scene_loaded)(const char* scene_path) = nullptr;
    void (*game_on_lua_script_loaded)(const char* script_path) = nullptr;
    void (*game_on_lua_script_reloaded)(const char* script_path) = nullptr;
    const char* (*game_get_version)() = nullptr;
    int (*game_get_build_number)() = nullptr;
    
    bool IsValid() const {
        return game_initialize && game_shutdown && game_update;
    }
};

// Asset Pack Runtime - Loads and manages packed assets
class AssetPackRuntime {
public:
    AssetPackRuntime() = default;
    ~AssetPackRuntime();

    bool Load(const std::filesystem::path& packFile);
    void Unload();

    // Asset retrieval
    const uint8_t* GetAssetData(const std::string& path, size_t& outSize);
    const uint8_t* GetAssetDataByHash(uint64_t pathHash, size_t& outSize);
    
    // Lua script retrieval
    std::vector<LuaScriptEntry> GetLuaScripts() const;
    const uint8_t* GetLuaScriptData(const std::string& path, size_t& outSize);

    // Info
    uint32_t GetAssetCount() const { return m_Header.asset_count; }
    uint32_t GetLuaScriptCount() const { return m_Header.lua_script_count; }
    bool IsLoaded() const { return m_Loaded; }

private:
    bool LoadHeader(std::ifstream& file);
    bool LoadIndexTable(std::ifstream& file);
    bool LoadLuaScriptIndex(std::ifstream& file);
    bool LoadAssetData(std::ifstream& file);

private:
    AssetPackHeader m_Header = {};
    std::vector<AssetIndexEntry> m_AssetIndex;
    std::vector<LuaScriptIndexEntry> m_LuaScriptIndex;
    std::vector<uint8_t> m_AssetData;
    std::unordered_map<uint64_t, size_t> m_HashToIndexMap;
    std::unordered_map<uint64_t, size_t> m_LuaHashToIndexMap;
    bool m_Loaded = false;
};

// Game DLL Manager - Loads and manages Game.dll
class GameDLLManager {
public:
    GameDLLManager() = default;
    ~GameDLLManager();

    // DLL operations
    bool LoadDLL(const std::filesystem::path& dllPath);
    void UnloadDLL();
    bool ReloadDLL(); // Hot-reload support
    
    // API access
    const GameAPI& GetAPI() const { return m_API; }
    GameAPI& GetAPI() { return m_API; }
    bool IsLoaded() const { return m_DLLHandle != nullptr; }
    
    // Search for DLL in common locations
    static std::filesystem::path FindGameDLL();

private:
    bool ResolveAPI();
    void* GetFunctionAddress(const char* name);

private:
    void* m_DLLHandle = nullptr;
    GameAPI m_API = {};
    std::filesystem::path m_DLLPath;
};

// Hybrid Build System Manager - Orchestrates DLL + Asset Pack
class HybridBuildSystem {
public:
    HybridBuildSystem() = default;
    ~HybridBuildSystem();

    // Initialization
    bool Initialize(void* engineContext, sol::state* luaState);
    void Shutdown();

    // Component access
    AssetPackRuntime* GetAssetPack() { return m_AssetPack.get(); }
    GameDLLManager* GetGameDLL() { return m_GameDLL.get(); }
    const GameAPI& GetGameAPI() const { return m_GameDLL->GetAPI(); }

    // Runtime operations
    void Update(float deltaTime);
    void Render();
    
    // Lua script loading from asset pack
    bool LoadLuaScriptsFromAssetPack(sol::state& luaState);
    bool ReloadLuaScripts(sol::state& luaState);

    // Event forwarding
    void OnKeyPressed(int keyCode);
    void OnKeyReleased(int keyCode);
    void OnMouseMoved(float x, float y);
    void OnMouseButton(int button, int action);

    // Status
    bool IsReady() const { return m_Initialized && m_AssetPack->IsLoaded() && m_GameDLL->IsLoaded(); }

    // Search for asset pack in common locations
    static std::filesystem::path FindAssetPack();

private:
    Scope<AssetPackRuntime> m_AssetPack;
    Scope<GameDLLManager> m_GameDLL;
    void* m_EngineContext = nullptr;
    sol::state* m_LuaState = nullptr;
    bool m_Initialized = false;
};

} // namespace S67
