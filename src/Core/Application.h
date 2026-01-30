#pragma once

#include "Base.h"
#include "GameState.h"
#include "UndoSystem.h"

#include "../Events/WindowEvent.h"
#include "../ImGui/ImGuiLayer.h"
#include "../ImGui/Panels/SceneHierarchyPanel.h"
#include "../Physics/PhysicsSystem.h"
#include "../Renderer/Camera.h"
#include "../Renderer/CameraController.h"
#include "../Renderer/Framebuffer.h"
#include "../Renderer/HUDRenderer.h"
#include "../Renderer/Light.h"
#include "../Renderer/Scene.h"
#include "../Renderer/Shader.h"
#include "../Renderer/Skybox.h"
#include "../Renderer/Texture.h"
#include "../Renderer/VertexArray.h"
#include "Window.h"

#include <filesystem>
#include <glad/glad.h>
#include <memory>
#include <string>
#include <vector>

namespace S67 {

class ContentBrowserPanel;
class ConsolePanel;

struct GameAPI {
  void (*game_initialize)(void *lua_state);
  void (*game_shutdown)();
  void (*game_update)(float dt);
  void (*game_render)();
  void (*game_load_assets)(const char *pak_path);
  void (*game_on_key_pressed)(int key_code);
  void (*game_on_key_released)(int key_code);
  void (*game_on_mouse_moved)(float x, float y);
  void (*game_on_mouse_button)(int button, int action);
  const char *(*game_get_name)();
  const char *(*game_get_version)();
};

class Application {
public:
  Application(const std::string &executablePath, const std::string &arg = "");
  virtual ~Application();

  void Run();
  void OnEvent(Event &e);

  void PushLayer(Layer *layer);
  void PushOverlay(Layer *layer);

  inline Window &GetWindow() { return *m_Window; }
  static inline Application &Get() { return *s_Instance; }

  // Scene Management
  void OnScenePlay();
  void OnScenePause();
  void OnSceneStop();

  void SetProjectRoot(const std::filesystem::path &root);
  const std::filesystem::path &GetProjectRoot() const { return m_ProjectRoot; }

  void OnNewProject();
  void OnOpenProject();
  void DiscoverProject(const std::filesystem::path &levelPath);
  void SaveManifest();

  void OnNewScene();
  void OnSaveScene();
  void OnSaveSceneAs();
  void OnOpenScene();
  void OpenScene(const std::string &filepath);
  void CloseScene();
  void CloseProject();

  void OnEntityCollidableChanged(Ref<Entity> entity);

  // Tick System
  void SetTickRate(float rate);
  float GetTickRate() const { return m_TickRate; }
  uint64_t GetTickNumber() const { return m_TickNumber; }

  float GetGameFPS() const { return m_GameFPS; }

  // Game DLL Management
  std::string FindGameDLL();
  std::string FindAssetPack();
  void *LoadGameDLL(const std::string &dllPath);
  void UnloadGameDLL(void *handle);
  GameAPI ResolveGameAPI(void *handle);

private:
  void RenderFrame(float alpha);
  void UpdateGameTick(float tick_dt);

  bool OnWindowClose(WindowCloseEvent &e);
  bool OnWindowResize(WindowResizeEvent &e);
  bool OnWindowDrop(WindowDropEvent &e);

#ifdef S67_EDITOR
  void UI_SettingsWindow();
  void UI_ProjectSettingsWindow();
  void UI_LauncherScreen();
#endif

  void AddToRecentProjects(const std::string &path);
  void InitDefaultAssets();

  void SaveSettings();
  void LoadSettings();
  void SaveLayout();
  void LoadLayout();
  void SaveLayout(const std::string &path);
  void LoadLayout(const std::string &path);
  void ResetLayout();

  std::unique_ptr<Window> m_Window;
  bool m_Running = true;

  // Tick System Constants
  float m_TickRate = 66.0f;
  float m_TickDuration = 1.0f / 66.0f;
  static constexpr float MAX_FRAME_TIME = 0.25f;

  // Tick System State
  GameState m_CurrentState;
  GameState m_PreviousState;
  double m_Accumulator = 0.0;
  double m_PreviousFrameTime = 0.0;
  uint64_t m_TickNumber = 0;

  Ref<PerspectiveCamera> m_Camera;
  Ref<PerspectiveCamera> m_EditorCamera;
  Ref<CameraController> m_CameraController;
  Ref<CameraController> m_EditorCameraController;
  Scope<Scene> m_Scene;
  DirectionalLight m_Sun;

  float m_LastFrameTime = 0.0f;
  float m_GameFPS = 0.0f;

  // Editor State
  enum class SceneState { Edit = 0, Play = 1, Pause = 2 };
  SceneState m_SceneState = SceneState::Edit;
  bool m_SceneViewportFocused = false;
  bool m_SceneViewportHovered = false;
  bool m_GameViewportFocused = false;
  bool m_GameViewportHovered = false;

  bool m_ShowInspector = true;
  bool m_ShowHierarchy = true;
  bool m_ShowContentBrowser = true;
  bool m_ShowConsole = true;
  bool m_ShowSettingsWindow = false;
  bool m_ShowProjectSettingsWindow = false;
  bool m_ShowScene = true;
  bool m_ShowGame = true;
  bool m_EnableConsole = true;

  // DLL Management
  void *m_GameDLLHandle = nullptr;
  GameAPI m_GameAPI;

  // VFS / Asset Management
  std::filesystem::path m_ProjectRoot;
  std::filesystem::path m_EngineAssetsRoot;
  std::vector<std::string> m_RecentProjects;

  // Renderer Resources
  Ref<Framebuffer> m_SceneFramebuffer;
  Ref<Framebuffer> m_GameFramebuffer;
  Ref<Shader> m_OutlineShader;
  Ref<Skybox> m_Skybox;
  Ref<Texture2D> m_LauncherLogo;

  // UI Panels
  Scope<SceneHierarchyPanel> m_SceneHierarchyPanel;
  Scope<ContentBrowserPanel> m_ContentBrowserPanel;
  Scope<ConsolePanel> m_ConsolePanel;

  // ImGui
  ImGuiLayer *m_ImGuiLayer;
  UndoSystem m_UndoSystem;

  bool m_SceneModified = false;
  bool m_ShowSaveNotification = false;
  float m_SaveNotificationTime = 0.0f;

  std::string m_ProjectName = "New Project";
  std::string m_ProjectVersion = "1.0.0";
  std::string m_ProjectCompany = "Default Company";
  std::string m_ProjectDefaultLevel = "";

  // Singleton
  static Application *s_Instance;

  friend class ContentBrowserPanel;
  friend class SceneHierarchyPanel;
};

} // namespace S67
