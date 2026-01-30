#include "HybridBuildSystem.h"
#include "Logger.h"
#include <fstream>
#include <cstring>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

namespace S67 {

// ============================================================================
// AssetPackRuntime Implementation
// ============================================================================

AssetPackRuntime::~AssetPackRuntime() {
    Unload();
}

bool AssetPackRuntime::Load(const std::filesystem::path& packFile) {
    S67_CORE_INFO("Loading asset pack: {}", packFile.string());

    std::ifstream file(packFile, std::ios::binary);
    if (!file) {
        S67_CORE_ERROR("Failed to open asset pack file: {}", packFile.string());
        return false;
    }

    // Load header
    if (!LoadHeader(file)) {
        return false;
    }

    // Load asset data
    if (!LoadAssetData(file)) {
        return false;
    }

    // Load index table
    if (!LoadIndexTable(file)) {
        return false;
    }

    // Load Lua script index
    if (!LoadLuaScriptIndex(file)) {
        return false;
    }

    m_Loaded = true;
    S67_CORE_INFO("Asset pack loaded successfully!");
    S67_CORE_INFO("  Total assets: {}", m_Header.asset_count);
    S67_CORE_INFO("  Lua scripts: {}", m_Header.lua_script_count);

    return true;
}

void AssetPackRuntime::Unload() {
    m_AssetData.clear();
    m_AssetIndex.clear();
    m_LuaScriptIndex.clear();
    m_HashToIndexMap.clear();
    m_LuaHashToIndexMap.clear();
    m_Loaded = false;
}

bool AssetPackRuntime::LoadHeader(std::ifstream& file) {
    file.read(reinterpret_cast<char*>(&m_Header), sizeof(m_Header));
    
    if (m_Header.magic != ASSETPACK_MAGIC) {
        S67_CORE_ERROR("Invalid asset pack magic number: 0x{:X}", m_Header.magic);
        return false;
    }

    if (m_Header.version != ASSETPACK_VERSION) {
        S67_CORE_ERROR("Unsupported asset pack version: {}", m_Header.version);
        return false;
    }

    return true;
}

bool AssetPackRuntime::LoadAssetData(std::ifstream& file) {
    // Calculate total asset data size
    uint64_t dataSize = 0;
    
    // We need to seek to index to calculate data size
    // For now, read until index offset
    if (m_Header.index_offset > sizeof(AssetPackHeader)) {
        dataSize = m_Header.index_offset - sizeof(AssetPackHeader);
        m_AssetData.resize(dataSize);
        
        file.seekg(sizeof(AssetPackHeader));
        file.read(reinterpret_cast<char*>(m_AssetData.data()), dataSize);
        
        if (!file) {
            S67_CORE_ERROR("Failed to read asset data");
            return false;
        }
    }

    return true;
}

bool AssetPackRuntime::LoadIndexTable(std::ifstream& file) {
    if (m_Header.asset_count == 0) {
        return true;
    }

    file.seekg(m_Header.index_offset);
    
    m_AssetIndex.resize(m_Header.asset_count);
    file.read(reinterpret_cast<char*>(m_AssetIndex.data()), 
              sizeof(AssetIndexEntry) * m_Header.asset_count);

    if (!file) {
        S67_CORE_ERROR("Failed to read asset index table");
        return false;
    }

    // Build hash map for faster lookups
    for (size_t i = 0; i < m_AssetIndex.size(); ++i) {
        m_HashToIndexMap[m_AssetIndex[i].path_hash] = i;
    }

    return true;
}

bool AssetPackRuntime::LoadLuaScriptIndex(std::ifstream& file) {
    if (m_Header.lua_script_count == 0) {
        return true;
    }

    // Lua script index comes after asset index
    uint64_t luaIndexOffset = m_Header.index_offset + 
                               (sizeof(AssetIndexEntry) * m_Header.asset_count);
    
    file.seekg(luaIndexOffset);
    
    m_LuaScriptIndex.resize(m_Header.lua_script_count);
    file.read(reinterpret_cast<char*>(m_LuaScriptIndex.data()),
              sizeof(LuaScriptIndexEntry) * m_Header.lua_script_count);

    if (!file) {
        S67_CORE_ERROR("Failed to read Lua script index");
        return false;
    }

    // Build hash map for faster lookups
    for (size_t i = 0; i < m_LuaScriptIndex.size(); ++i) {
        m_LuaHashToIndexMap[m_LuaScriptIndex[i].path_hash] = i;
    }

    return true;
}

const uint8_t* AssetPackRuntime::GetAssetData(const std::string& path, size_t& outSize) {
    uint64_t hash = HashString(path);
    return GetAssetDataByHash(hash, outSize);
}

const uint8_t* AssetPackRuntime::GetAssetDataByHash(uint64_t pathHash, size_t& outSize) {
    auto it = m_HashToIndexMap.find(pathHash);
    if (it == m_HashToIndexMap.end()) {
        outSize = 0;
        return nullptr;
    }

    const AssetIndexEntry& entry = m_AssetIndex[it->second];
    outSize = entry.size;
    
    // Offset is from beginning of file, adjust for our data buffer
    uint64_t dataOffset = entry.offset - sizeof(AssetPackHeader);
    
    if (dataOffset + entry.size > m_AssetData.size()) {
        S67_CORE_ERROR("Asset data out of bounds");
        outSize = 0;
        return nullptr;
    }

    return m_AssetData.data() + dataOffset;
}

std::vector<LuaScriptEntry> AssetPackRuntime::GetLuaScripts() const {
    std::vector<LuaScriptEntry> scripts;
    scripts.reserve(m_LuaScriptIndex.size());

    for (const auto& indexEntry : m_LuaScriptIndex) {
        LuaScriptEntry script;
        script.path_hash = indexEntry.path_hash;
        script.checksum = indexEntry.checksum;
        
        // Get data from asset data buffer
        uint64_t dataOffset = indexEntry.offset - sizeof(AssetPackHeader);
        if (dataOffset + indexEntry.size <= m_AssetData.size()) {
            script.data.resize(indexEntry.size);
            std::memcpy(script.data.data(), 
                       m_AssetData.data() + dataOffset, 
                       indexEntry.size);
        }

        scripts.push_back(std::move(script));
    }

    return scripts;
}

const uint8_t* AssetPackRuntime::GetLuaScriptData(const std::string& path, size_t& outSize) {
    uint64_t hash = HashString(path);
    
    auto it = m_LuaHashToIndexMap.find(hash);
    if (it == m_LuaHashToIndexMap.end()) {
        outSize = 0;
        return nullptr;
    }

    const LuaScriptIndexEntry& entry = m_LuaScriptIndex[it->second];
    outSize = entry.size;
    
    // Offset is from beginning of file, adjust for our data buffer
    uint64_t dataOffset = entry.offset - sizeof(AssetPackHeader);
    
    if (dataOffset + entry.size > m_AssetData.size()) {
        S67_CORE_ERROR("Lua script data out of bounds");
        outSize = 0;
        return nullptr;
    }

    return m_AssetData.data() + dataOffset;
}

// ============================================================================
// GameDLLManager Implementation
// ============================================================================

GameDLLManager::~GameDLLManager() {
    UnloadDLL();
}

bool GameDLLManager::LoadDLL(const std::filesystem::path& dllPath) {
    S67_CORE_INFO("Loading Game DLL: {}", dllPath.string());

    if (!std::filesystem::exists(dllPath)) {
        S67_CORE_ERROR("Game DLL not found: {}", dllPath.string());
        return false;
    }

    m_DLLPath = dllPath;

#ifdef _WIN32
    m_DLLHandle = LoadLibraryA(dllPath.string().c_str());
    if (!m_DLLHandle) {
        S67_CORE_ERROR("Failed to load Game DLL: {} (Error: {})", 
                       dllPath.string(), GetLastError());
        return false;
    }
#else
    m_DLLHandle = dlopen(dllPath.string().c_str(), RTLD_LAZY);
    if (!m_DLLHandle) {
        S67_CORE_ERROR("Failed to load Game DLL: {} ({})", 
                       dllPath.string(), dlerror());
        return false;
    }
#endif

    if (!ResolveAPI()) {
        UnloadDLL();
        return false;
    }

    S67_CORE_INFO("Game DLL loaded successfully!");
    return true;
}

void GameDLLManager::UnloadDLL() {
    if (m_DLLHandle) {
        S67_CORE_INFO("Unloading Game DLL");
        
#ifdef _WIN32
        FreeLibrary(static_cast<HMODULE>(m_DLLHandle));
#else
        dlclose(m_DLLHandle);
#endif
        m_DLLHandle = nullptr;
    }
    
    m_API = {};
}

bool GameDLLManager::ReloadDLL() {
    if (!m_DLLHandle) {
        return false;
    }

    auto path = m_DLLPath;
    UnloadDLL();
    return LoadDLL(path);
}

void* GameDLLManager::GetFunctionAddress(const char* name) {
#ifdef _WIN32
    return reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(m_DLLHandle), name));
#else
    return dlsym(m_DLLHandle, name);
#endif
}

bool GameDLLManager::ResolveAPI() {
    S67_CORE_INFO("Resolving Game API functions...");

    #define RESOLVE_FUNC(name) \
        m_API.name = reinterpret_cast<decltype(m_API.name)>(GetFunctionAddress(#name)); \
        if (!m_API.name) { \
            S67_CORE_WARN("Failed to resolve function: {}", #name); \
        }

    RESOLVE_FUNC(game_initialize);
    RESOLVE_FUNC(game_shutdown);
    RESOLVE_FUNC(game_update);
    RESOLVE_FUNC(game_render);
    RESOLVE_FUNC(game_on_key_pressed);
    RESOLVE_FUNC(game_on_key_released);
    RESOLVE_FUNC(game_on_mouse_moved);
    RESOLVE_FUNC(game_on_mouse_button);
    RESOLVE_FUNC(game_on_assets_loaded);
    RESOLVE_FUNC(game_on_scene_loaded);
    RESOLVE_FUNC(game_on_lua_script_loaded);
    RESOLVE_FUNC(game_on_lua_script_reloaded);
    RESOLVE_FUNC(game_get_version);
    RESOLVE_FUNC(game_get_build_number);

    #undef RESOLVE_FUNC

    if (!m_API.IsValid()) {
        S67_CORE_ERROR("Failed to resolve required Game API functions");
        return false;
    }

    S67_CORE_INFO("Game API resolved successfully!");
    
    if (m_API.game_get_version) {
        S67_CORE_INFO("  Game Version: {}", m_API.game_get_version());
    }
    if (m_API.game_get_build_number) {
        S67_CORE_INFO("  Build Number: {}", m_API.game_get_build_number());
    }

    return true;
}

std::filesystem::path GameDLLManager::FindGameDLL() {
    std::vector<std::filesystem::path> searchPaths = {
        "Game.dll",
        "game/build/Release/Game.dll",
        "game/build/Debug/Game.dll",
        "../game/build/Release/Game.dll",
        "../game/build/Debug/Game.dll",
#ifndef _WIN32
        "libGame.so",
        "game/build/Release/libGame.so",
        "game/build/Debug/libGame.so",
        "libGame.dylib",
        "game/build/Release/libGame.dylib",
        "game/build/Debug/libGame.dylib",
#endif
    };

    // Check environment variable
    if (const char* envPath = std::getenv("GAME_DLL_PATH")) {
        searchPaths.insert(searchPaths.begin(), envPath);
    }

    for (const auto& path : searchPaths) {
        if (std::filesystem::exists(path)) {
            return std::filesystem::absolute(path);
        }
    }

    return {};
}

// ============================================================================
// HybridBuildSystem Implementation
// ============================================================================

HybridBuildSystem::~HybridBuildSystem() {
    Shutdown();
}

bool HybridBuildSystem::Initialize(void* engineContext, sol::state* luaState) {
    S67_CORE_INFO("Initializing Hybrid Build System...");

    m_EngineContext = engineContext;
    m_LuaState = luaState;

    // Create components
    m_AssetPack = CreateScope<AssetPackRuntime>();
    m_GameDLL = CreateScope<GameDLLManager>();

    // Find and load asset pack
    auto assetPackPath = FindAssetPack();
    if (assetPackPath.empty()) {
        S67_CORE_WARN("Asset pack not found - running without packed assets");
    } else {
        if (!m_AssetPack->Load(assetPackPath)) {
            S67_CORE_ERROR("Failed to load asset pack");
            return false;
        }
    }

    // Find and load game DLL
    auto gameDLLPath = GameDLLManager::FindGameDLL();
    if (gameDLLPath.empty()) {
        S67_CORE_WARN("Game DLL not found - running without game code");
    } else {
        if (!m_GameDLL->LoadDLL(gameDLLPath)) {
            S67_CORE_ERROR("Failed to load Game DLL");
            return false;
        }
    }

    // Initialize game if DLL loaded
    if (m_GameDLL->IsLoaded() && m_GameDLL->GetAPI().game_initialize) {
        m_GameDLL->GetAPI().game_initialize(engineContext, luaState);
    }

    // Load Lua scripts from asset pack
    if (m_AssetPack->IsLoaded() && luaState) {
        LoadLuaScriptsFromAssetPack(*luaState);
    }

    // Notify game that assets are loaded
    if (m_GameDLL->IsLoaded() && m_AssetPack->IsLoaded() && 
        m_GameDLL->GetAPI().game_on_assets_loaded) {
        m_GameDLL->GetAPI().game_on_assets_loaded(m_AssetPack.get());
    }

    m_Initialized = true;
    S67_CORE_INFO("Hybrid Build System initialized successfully!");

    return true;
}

void HybridBuildSystem::Shutdown() {
    if (!m_Initialized) {
        return;
    }

    S67_CORE_INFO("Shutting down Hybrid Build System...");

    // Shutdown game DLL
    if (m_GameDLL && m_GameDLL->IsLoaded() && m_GameDLL->GetAPI().game_shutdown) {
        m_GameDLL->GetAPI().game_shutdown();
    }

    m_GameDLL.reset();
    m_AssetPack.reset();
    
    m_Initialized = false;
}

void HybridBuildSystem::Update(float deltaTime) {
    if (m_GameDLL && m_GameDLL->IsLoaded() && m_GameDLL->GetAPI().game_update) {
        m_GameDLL->GetAPI().game_update(deltaTime);
    }
}

void HybridBuildSystem::Render() {
    if (m_GameDLL && m_GameDLL->IsLoaded() && m_GameDLL->GetAPI().game_render) {
        m_GameDLL->GetAPI().game_render();
    }
}

bool HybridBuildSystem::LoadLuaScriptsFromAssetPack(sol::state& luaState) {
    if (!m_AssetPack || !m_AssetPack->IsLoaded()) {
        return false;
    }

    S67_CORE_INFO("Loading Lua scripts from asset pack...");

    auto scripts = m_AssetPack->GetLuaScripts();
    
    for (const auto& script : scripts) {
        try {
            std::string scriptCode(reinterpret_cast<const char*>(script.data.data()), 
                                  script.data.size());
            
            // Execute script
            luaState.script(scriptCode);
            
            S67_CORE_INFO("Loaded Lua script (hash: 0x{:X})", script.path_hash);
            
            // Notify game DLL
            if (m_GameDLL && m_GameDLL->IsLoaded() && 
                m_GameDLL->GetAPI().game_on_lua_script_loaded) {
                std::string scriptPath = "script_" + std::to_string(script.path_hash);
                m_GameDLL->GetAPI().game_on_lua_script_loaded(scriptPath.c_str());
            }
        }
        catch (const sol::error& e) {
            S67_CORE_ERROR("Failed to load Lua script: {}", e.what());
        }
    }

    S67_CORE_INFO("Loaded {} Lua scripts", scripts.size());
    return true;
}

bool HybridBuildSystem::ReloadLuaScripts(sol::state& luaState) {
    // Reload all Lua scripts from asset pack
    return LoadLuaScriptsFromAssetPack(luaState);
}

void HybridBuildSystem::OnKeyPressed(int keyCode) {
    if (m_GameDLL && m_GameDLL->IsLoaded() && m_GameDLL->GetAPI().game_on_key_pressed) {
        m_GameDLL->GetAPI().game_on_key_pressed(keyCode);
    }
}

void HybridBuildSystem::OnKeyReleased(int keyCode) {
    if (m_GameDLL && m_GameDLL->IsLoaded() && m_GameDLL->GetAPI().game_on_key_released) {
        m_GameDLL->GetAPI().game_on_key_released(keyCode);
    }
}

void HybridBuildSystem::OnMouseMoved(float x, float y) {
    if (m_GameDLL && m_GameDLL->IsLoaded() && m_GameDLL->GetAPI().game_on_mouse_moved) {
        m_GameDLL->GetAPI().game_on_mouse_moved(x, y);
    }
}

void HybridBuildSystem::OnMouseButton(int button, int action) {
    if (m_GameDLL && m_GameDLL->IsLoaded() && m_GameDLL->GetAPI().game_on_mouse_button) {
        m_GameDLL->GetAPI().game_on_mouse_button(button, action);
    }
}

std::filesystem::path HybridBuildSystem::FindAssetPack() {
    std::vector<std::filesystem::path> searchPaths = {
        "GameAssets.apak",
        "assets/GameAssets.apak",
        "../assets/GameAssets.apak",
        "build/GameAssets.apak",
        "../build/GameAssets.apak",
    };

    // Check environment variable
    if (const char* envPath = std::getenv("ASSETPACK_PATH")) {
        searchPaths.insert(searchPaths.begin(), envPath);
    }

    for (const auto& path : searchPaths) {
        if (std::filesystem::exists(path)) {
            return std::filesystem::absolute(path);
        }
    }

    return {};
}

} // namespace S67
