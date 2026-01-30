#define GAME_DLL_EXPORT
#include "game_api.h"

#include <iostream>
#include <string>

// Note: In production, these would include Source67 headers
// For now, we use minimal dependencies to avoid coupling

// Global state for the game DLL
namespace GameDLL {
    static void* g_EngineContext = nullptr;
    static void* g_LuaState = nullptr;
    static void* g_AssetPack = nullptr;
    static std::string g_CurrentScene;
}

// ============================================================================
// API Implementation
// ============================================================================

extern "C" {

GAME_API void game_initialize(void* engine_context, void* lua_state) {
    GameDLL::g_EngineContext = engine_context;
    GameDLL::g_LuaState = lua_state;
    
    std::cout << "[Game DLL] Initialized!" << std::endl;
    std::cout << "[Game DLL] Engine Context: " << engine_context << std::endl;
    std::cout << "[Game DLL] Lua State: " << lua_state << std::endl;
}

GAME_API void game_shutdown() {
    std::cout << "[Game DLL] Shutting down..." << std::endl;
    GameDLL::g_EngineContext = nullptr;
    GameDLL::g_LuaState = nullptr;
    GameDLL::g_AssetPack = nullptr;
}

GAME_API void game_update(float delta_time) {
    // Game logic update
    // In production: Update game systems, call native scripts, etc.
    static float totalTime = 0.0f;
    totalTime += delta_time;
    
    // Example: Print every 60 frames (at 60 FPS = 1 second)
    static int frameCount = 0;
    if (++frameCount % 60 == 0) {
        std::cout << "[Game DLL] Update - Time: " << totalTime << "s" << std::endl;
    }
}

GAME_API void game_render() {
    // Custom game rendering
    // In production: Queue custom render commands, draw HUD, etc.
}

GAME_API void game_on_key_pressed(int key_code) {
    std::cout << "[Game DLL] Key pressed: " << key_code << std::endl;
}

GAME_API void game_on_key_released(int key_code) {
    std::cout << "[Game DLL] Key released: " << key_code << std::endl;
}

GAME_API void game_on_mouse_moved(float x, float y) {
    // Mouse movement handling
}

GAME_API void game_on_mouse_button(int button, int action) {
    std::cout << "[Game DLL] Mouse button: " << button << " action: " << action << std::endl;
}

GAME_API void game_on_assets_loaded(void* assetpack_handle) {
    GameDLL::g_AssetPack = assetpack_handle;
    std::cout << "[Game DLL] Assets loaded - Handle: " << assetpack_handle << std::endl;
    
    // In production: Load initial scene
    // game_on_scene_loaded("scenes/main.s67");
}

GAME_API void game_on_scene_loaded(const char* scene_path) {
    GameDLL::g_CurrentScene = scene_path;
    std::cout << "[Game DLL] Scene loaded: " << scene_path << std::endl;
}

GAME_API void game_on_lua_script_loaded(const char* script_path) {
    std::cout << "[Game DLL] Lua script loaded: " << script_path << std::endl;
    
    // In production: Execute Lua script
    // if (GameDLL::g_LuaState) {
    //     sol::state* lua = static_cast<sol::state*>(GameDLL::g_LuaState);
    //     try {
    //         lua->script_file(script_path);
    //     } catch (const std::exception& e) {
    //         std::cerr << "Lua error: " << e.what() << std::endl;
    //     }
    // }
}

GAME_API void game_on_lua_script_reloaded(const char* script_path) {
    std::cout << "[Game DLL] Lua script reloaded: " << script_path << std::endl;
}

GAME_API const char* game_get_version() {
    return "1.0.0";
}

GAME_API int game_get_build_number() {
    return 1;
}

} // extern "C"
