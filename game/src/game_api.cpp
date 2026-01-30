#include "game_api.h"
#include <iostream>

// Global state for Example Game
static void *g_EngineContext = nullptr;
static void *g_LuaState = nullptr;

GAME_API void game_initialize(void *engine_context, void *lua_state) {
  g_EngineContext = engine_context;
  g_LuaState = lua_state;
  std::cout << "[Game DLL] Initialized" << std::endl;
}

GAME_API void game_update(float delta_time) {
  // Basic gameplay logic would go here
}

GAME_API void game_render() {
  // Custom render calls if needed
}

GAME_API void game_on_key_pressed(int key_code) {
  std::cout << "[Game DLL] Key Pressed: " << key_code << std::endl;
}

GAME_API void game_on_key_released(int key_code) {}
GAME_API void game_on_mouse_moved(float x, float y) {}
GAME_API void game_on_mouse_button(int button, int action) {}

GAME_API void game_on_assets_loaded(void *assetpack_handle) {
  std::cout << "[Game DLL] Assets Loaded" << std::endl;
}

GAME_API void game_on_scene_loaded(const char *scene_path) {
  std::cout << "[Game DLL] Scene Loaded: " << scene_path << std::endl;
}

GAME_API void game_on_lua_script_loaded(const char *script_path) {}
GAME_API void game_on_lua_script_reloaded(const char *script_path) {}

GAME_API void game_shutdown() {
  std::cout << "[Game DLL] Shutdown" << std::endl;
}

GAME_API const char *game_get_version() { return "1.0.0"; }

GAME_API int game_get_build_number() { return 1; }
