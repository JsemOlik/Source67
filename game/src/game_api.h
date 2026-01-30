#pragma once

#ifdef _WIN32
#define GAME_API __declspec(dllexport)
#else
#define GAME_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Initialization with engine context and Lua state
GAME_API void game_initialize(void *engine_context, void *lua_state);

// Core game loop
GAME_API void game_update(float delta_time);
GAME_API void game_render();

// Input callbacks
GAME_API void game_on_key_pressed(int key_code);
GAME_API void game_on_key_released(int key_code);
GAME_API void game_on_mouse_moved(float x, float y);
GAME_API void game_on_mouse_button(int button, int action);

// Asset and scene loading
GAME_API void game_on_assets_loaded(void *assetpack_handle);
GAME_API void game_on_scene_loaded(const char *scene_path);

// Lua integration
GAME_API void game_on_lua_script_loaded(const char *script_path);
GAME_API void game_on_lua_script_reloaded(const char *script_path);

// Lifecycle
GAME_API void game_shutdown();

// Metadata
GAME_API const char *game_get_version();
GAME_API int game_get_build_number();

#ifdef __cplusplus
}
#endif
