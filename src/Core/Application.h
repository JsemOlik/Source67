#include "Base.h"
#include "Core/GameState.h"
#include "Core/HybridBuildSystem.h"
#include "Core/UndoSystem.h"
#include "Events/WindowEvent.h"
#include "ImGui/ImGuiLayer.h"
#include "ImGui/Panels/SceneHierarchyPanel.h"
#include "Physics/PhysicsSystem.h"
// #include "Physics/PlayerController.h" // Removed to break potential cycle
#include "Renderer/Camera.h"
#include "Renderer/CameraController.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/HUDRenderer.h"
#include "Renderer/Light.h"
#include "Renderer/Scene.h"
#include "Renderer/Shader.h"
#include "Renderer/Skybox.h"
#include "Renderer/Texture.h"
#include "Renderer/VertexArray.h"
#include "Window.h"
#include <filesystem>
#include <glad/glad.h>

namespace S67 {

class ContentBrowserPanel;
class ConsolePanel;

enum class SceneState { Edit = 0, Play = 1, Pause = 2 };

enum class EditorTheme { Unity = 0, Dracula = 1, Classic = 2, Light = 3 };

class Application {
public:
  Application(const std::string &executablePath, const std::string &arg = "");
  virtual ~Application();

  void Run();

  void OnEvent(Event &e);

  void SaveSettings();
  void LoadSettings();

  void SaveLayout();
  void LoadLayout();
  void SaveLayout(const std::string &path);
  void LoadLayout(const std::string &path);
  void ResetLayout();

  void OnScenePlay();
  void OnScenePause();
  void OnSceneStop();

  void OnSaveScene();                          // Quick Save (Shortcuts)
  void OnSaveSceneAs();                        // Dialog version
  void OnOpenScene();                          // Dialog version
  void OpenScene(const std::string &filepath); // Direct version
  void OnNewScene();
  void CloseScene();
  void CloseProject();

  void OnEntityCollidableChanged(Ref<Entity> entity);
  void SetSceneModified(bool modified) { m_SceneModified = modified; }

  void OnNewProject();
  void OnOpenProject();
  void DiscoverProject(const std::filesystem::path &levelPath);
  void SaveManifest();

  void CreateTestScene();

  // Build System Methods
  void OnBuildGame();
  void OnBuildAssets();
  void OnBuildAll();
  void OnPackageGame();

  // Tick System Methods
  void SetTickRate(float rate);
  void UpdateGameTick(float tick_dt);

  // Renders a single frame (useful for both Run loop and resize events)
  void RenderFrame(float alpha);

  inline Window &GetWindow() { return *m_Window; }
  ImGuiLayer &GetImGuiLayer() { return *m_ImGuiLayer; }
  inline static Application &Get() { return *s_Instance; }

  Ref<PerspectiveCamera> GetCamera() { return m_Camera; }
  Scene &GetScene() { return *m_Scene; }

  const std::filesystem::path &GetProjectRoot() const { return m_ProjectRoot; }
  void SetProjectRoot(const std::filesystem::path &root);

  UndoSystem &GetUndoSystem() { return m_UndoSystem; }

  Ref<Shader> GetDefaultShader() const { return m_DefaultShader; }
  Ref<Texture2D> GetDefaultTexture() const { return m_DefaultTexture; }
  Ref<VertexArray> GetCubeMesh() const { return m_CubeMesh; }

  std::filesystem::path ResolveAssetPath(const std::filesystem::path &path);
  const std::filesystem::path &GetEngineAssetsRoot() const {
    return m_EngineAssetsRoot;
  }

private:
  bool OnWindowClose(WindowCloseEvent &e);
  bool OnWindowResize(WindowResizeEvent &e);
  bool OnWindowDrop(WindowDropEvent &e);

  void UI_SettingsWindow();
  void UI_ProjectSettingsWindow();
  void UI_LauncherScreen();

  void AddToRecentProjects(const std::string &path);
  void InitDefaultAssets();

  std::unique_ptr<Window> m_Window;
  bool m_Running = true;

  // Tick System Constants
  float m_TickRate = 66.0f;
  float m_TickDuration = 1.0f / 66.0f;
  static constexpr float MAX_FRAME_TIME =
      0.25f; // Max 250ms per frame (prevents spiral of death)

  // Tick System State
  GameState m_CurrentState;
  GameState m_PreviousState;
  double m_Accumulator = 0.0;
  double m_PreviousFrameTime = 0.0;
  uint64_t m_TickNumber = 0;

  Ref<PerspectiveCamera> m_Camera; // Game Camera
  Ref<PerspectiveCamera> m_EditorCamera;
  Ref<CameraController> m_CameraController; // Game Camera Controller
  Ref<CameraController> m_EditorCameraController;
  Scope<Scene> m_Scene;
  DirectionalLight m_Sun;

  float m_LastFrameTime = 0.0f;

  // PlayerController *m_PlayerController = nullptr; // Removed

  Scope<ImGuiLayer> m_ImGuiLayer;
  Scope<SceneHierarchyPanel> m_SceneHierarchyPanel;
  Scope<ContentBrowserPanel> m_ContentBrowserPanel;
  Scope<ConsolePanel> m_ConsolePanel;
  Scope<Skybox> m_Skybox;

  std::filesystem::path m_ProjectRoot;
  std::filesystem::path m_ProjectFilePath;
  std::string m_ProjectName = "Standalone";
  std::string m_ProjectCompany = "Default Company";
  std::string m_ProjectVersion = "N/A";
  std::string m_ProjectDefaultLevel = "";
  bool m_LevelLoaded = false;
  std::string m_LevelFilePath = "";

  int m_GizmoType = 7; // ImGuizmo::TRANSLATE
  UndoSystem m_UndoSystem;
  Transform m_InitialGizmoTransform;
  bool m_IsDraggingGizmo = false;

  Ref<Framebuffer> m_SceneFramebuffer;
  Ref<Framebuffer> m_GameFramebuffer;
  Ref<Shader> m_OutlineShader;

  glm::vec2 m_SceneViewportSize = {0, 0};
  glm::vec2 m_GameViewportSize = {0, 0};

  glm::vec2 m_SceneViewportPos = {0, 0};

  bool m_SceneViewportFocused = false, m_SceneViewportHovered = false;
  bool m_GameViewportFocused = false, m_GameViewportHovered = false;

  bool m_ShowSettingsWindow = false;
  bool m_ShowProjectSettingsWindow = false;
  float m_FontSize = 18.0f;
  float m_EditorFOV = 45.0f;
  std::string m_FontPath = "assets/fonts/Roboto-Medium.ttf";
  glm::vec4 m_CustomColor = {0.1f, 0.105f, 0.11f, 1.0f};
  EditorTheme m_EditorTheme = EditorTheme::Dracula;
  int m_FPSCap = 0; // 0 = Unlimited
  bool m_VSync = true;
  std::filesystem::path m_EngineAssetsRoot;

  bool m_ShowInspector = true;
  bool m_ShowHierarchy = true;
  bool m_ShowContentBrowser = true;
  bool m_ShowScene = true;
  bool m_ShowGame = true;
  bool m_ShowToolbar = true;
  bool m_ShowStats = true;
  bool m_ShowConsole = false;
  bool m_EnableConsole = true;
  bool m_ResetLayoutOnNextFrame = false;

  Ref<Shader> m_DefaultShader;
  Ref<Texture2D> m_DefaultTexture;
  Ref<VertexArray> m_CubeMesh;
  Ref<Shader> m_HUDShader;

  SceneState m_SceneState = SceneState::Edit;
  bool m_CursorLocked = false;

  // Save notification
  bool m_ShowSaveNotification = false;
  float m_SaveNotificationTime = 0.0f;

  // Unsaved changes tracking
  bool m_SceneModified = false;
  float m_LastAutoSaveTime = 0.0f;
  std::string m_PendingScenePath;

  float m_LastGameTime = 0.0f;
  float m_GameFPS = 0.0f;

  std::vector<std::string> m_RecentProjects;
  Ref<Texture2D> m_LauncherLogo;

  // Hybrid Build System
  Scope<HybridBuildSystem> m_HybridBuildSystem;

  static Application *s_Instance;
};

} // namespace S67
