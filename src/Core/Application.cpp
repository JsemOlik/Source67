#include "Application.h"
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Logger.h"
#include "Renderer/Buffer.h"
#include "Renderer/Renderer.h"
#include "Renderer/Shader.h"
#include "Renderer/VertexArray.h"
#include <glad/glad.h>

#include "Core/Input.h"
#include "Core/PlatformUtils.h"
#include "Game/Console/ConVar.h"
#include "Game/Console/Console.h"
#ifndef S67_RUNTIME
#include "Game/Console/ConsolePanel.h"
#include "ImGui/Panels/ContentBrowserPanel.h"
#include "ImGuizmo/ImGuizmo.h"
#endif
#include "Physics/PhysicsShapes.h"
#include "Physics/PlayerController.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/SceneSerializer.h"
#include "Renderer/ScriptableEntity.h"
#include "Renderer/ScriptRegistry.h"
#include "Scripting/LuaScriptEngine.h"

#include <GLFW/glfw3.h>
#if defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>
#elif defined(_WIN32)
#undef GLFW_EXPOSE_NATIVE_COCOA
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#ifndef S67_RUNTIME
#include <ImGuizmo.h>
#include <imgui.h>
#include <imgui_internal.h>
#endif

#include <cstdlib> // std::system
#include <thread>  // std::thread

#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include "Renderer/Mesh.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/transform.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <thread>

namespace S67 {

struct SceneBackup {
  struct TransformData {
    glm::vec3 Position, Rotation, Scale;
  };
  std::unordered_map<void *, TransformData> Data;
};
static SceneBackup s_SceneBackup;

Application *Application::s_Instance = nullptr;

static ConVar* s_ClShowFPS = nullptr;
static ConVar* s_SvTickRate = nullptr;

Application::Application(const std::string &executablePath,
                         const std::string &arg) {
  S67_CORE_ASSERT(!s_Instance, "Application already exists!");
  s_Instance = this;

  // Register Console Commands
  s_ClShowFPS = new ConVar("cl_showfps", "0", FCVAR_ARCHIVE, "Draw FPS meter");
  Console::Get().RegisterConVar(s_ClShowFPS);

  s_SvTickRate = new ConVar("sv_tickrate", "66", FCVAR_NOTIFY | FCVAR_ARCHIVE, "Server tick rate", [](ConVar* var, const std::string& newVal) {
      try {
          float rate = std::stof(newVal);
          Application::Get().SetTickRate(rate);
      } catch(...) {}
  });
  Console::Get().RegisterConVar(s_SvTickRate);

  // Find assets root
  std::filesystem::path currentPath =
      std::filesystem::absolute(executablePath).parent_path();
  bool found = false;
  for (int i = 0; i < 5; i++) {
    if (std::filesystem::exists(currentPath / "assets")) {
      std::filesystem::current_path(currentPath);
      found = true;
      break;
    }
    if (currentPath.has_parent_path())
      currentPath = currentPath.parent_path();
    else
      break;
  }

  if (!found) {
    S67_CORE_ERROR(
        "Could not find 'assets' directory relative to executable path: {0}!",
        executablePath);
    m_EngineAssetsRoot =
        std::filesystem::absolute(executablePath).parent_path();
  } else {
    m_EngineAssetsRoot = currentPath;
    S67_CORE_INFO("Set working directory to project root: {0}",
                  currentPath.string());
  }

  S67_CORE_INFO("Initializing Window...");
  m_Window = std::unique_ptr<Window>(Window::Create());
  m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
  m_Window->SetIcon(ResolveAssetPath("assets/engine/level_icon.png").string());

  S67_CORE_INFO("Initializing Renderer...");
  Renderer::Init();

  S67_CORE_INFO("Initializing Physics...");
  PhysicsSystem::Init();

  m_Camera =
      CreateRef<PerspectiveCamera>(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
  m_Camera->SetPosition({0.0f, 2.0f, 8.0f});
  m_Camera->SetPosition({0.0f, 2.0f, 8.0f});
  // m_PlayerController managed by Scene Script now

  // Initialize tick system game state
  m_PreviousFrameTime = glfwGetTime();
  m_CurrentState.player_position =
      m_Camera->GetPosition() - glm::vec3(0.0f, 1.7f, 0.0f);
  m_CurrentState.yaw = -90.0f;
  m_CurrentState.pitch = 0.0f;
  m_PreviousState = m_CurrentState;

#ifndef S67_RUNTIME
  m_EditorCamera =
      CreateRef<PerspectiveCamera>(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
  m_EditorCamera->SetPosition({5.0f, 5.0f, 15.0f});
#endif

  m_Scene = CreateScope<Scene>();
  m_Sun.Direction = {-0.5f, -1.0f, -0.2f};
  m_Sun.Color = {1.0f, 0.95f, 0.8f}; // Warm sun color
  m_Sun.Intensity = 1.0f;

  m_CameraController = CreateRef<CameraController>(m_Camera);
#ifndef S67_RUNTIME
  m_EditorCameraController = CreateRef<CameraController>(m_EditorCamera);
  m_EditorCameraController->SetRotationEnabled(false); // Only via Right-Click
  m_Window->SetCursorLocked(false);
  m_CursorLocked = false;

  m_ImGuiLayer = CreateScope<ImGuiLayer>();
  m_ImGuiLayer->OnAttach();

  m_SceneHierarchyPanel = CreateScope<SceneHierarchyPanel>(m_Scene);
  m_ContentBrowserPanel = CreateScope<ContentBrowserPanel>();
  m_ConsolePanel = CreateScope<ConsolePanel>();
  m_Skybox = CreateScope<Skybox>(
      ResolveAssetPath("assets/textures/sky-3.png").string());
  LoadSettings();
#else
  m_Skybox = CreateScope<Skybox>(
      ResolveAssetPath("assets/textures/sky-3.png").string());
#endif

#ifndef S67_RUNTIME
  if (!std::filesystem::exists("imgui.ini")) {
    m_ResetLayoutOnNextFrame = true;
  }

  FramebufferSpecification fbSpec;
  fbSpec.Width = 1280;
  fbSpec.Height = 720;
  m_SceneFramebuffer = Framebuffer::Create(fbSpec);
  m_GameFramebuffer = Framebuffer::Create(fbSpec);
  m_OutlineShader = Shader::Create(
      ResolveAssetPath("assets/shaders/FlatColor.glsl").string());
#endif

  std::filesystem::path logoPath =
      ResolveAssetPath("assets/engine/engine_logo.png");
  if (std::filesystem::exists(logoPath)) {
    m_LauncherLogo = Texture2D::Create(logoPath.string());
  }

  InitDefaultAssets();

  // Load Game Configuration (ConVars)
  S67_CORE_INFO("Loading game configuration...");
  Console::Get().Load("game.cfg");

  // Initialize HUD Renderer
  S67_CORE_INFO("Initializing HUD Renderer...");
  HUDRenderer::Init();

  // Initialize Scripting
  S67_CORE_INFO("Initializing Lua Engine...");
  LuaScriptEngine::Init();
  
  // Script loading is now deferred to SetProjectRoot or DiscoverProject

  m_HUDShader =
      Shader::Create(ResolveAssetPath("assets/shaders/HUD.glsl").string());
  HUDRenderer::SetShader(m_HUDShader);

#ifdef S67_RUNTIME
  // RUNTIME INITIALIZATION
  std::filesystem::path exePath =
      std::filesystem::absolute(executablePath).parent_path();
  S67_CORE_INFO("Runtime Startup at: {0}", exePath.string());

  // Try to find manifest
  std::filesystem::path runtimeManifest = exePath / "manifest.source";
  if (std::filesystem::exists(runtimeManifest)) {
    DiscoverProject(runtimeManifest);

    // Auto-load Default Level
    if (!m_ProjectDefaultLevel.empty()) {
      std::filesystem::path defaultLevelPath =
          ResolveAssetPath(m_ProjectDefaultLevel);
      if (std::filesystem::exists(defaultLevelPath)) {
        S67_CORE_INFO("Runtime loading default level: {0}",
                      defaultLevelPath.string());
        OpenScene(defaultLevelPath.string());
        OnScenePlay(); // Start playing immediately
      }
    }
  } else {
    S67_CORE_ERROR("No manifest.source found for Runtime!");
  }
#else
  if (!arg.empty()) {
    std::string cleanArg = arg;
    if (cleanArg.front() == '\"' && cleanArg.back() == '\"') {
      cleanArg = cleanArg.substr(1, cleanArg.length() - 2);
    }

    std::filesystem::path p(cleanArg);
    std::string ext = p.extension().string();
    for (auto &c : ext)
      c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    if (ext == ".s67") {
      S67_CORE_INFO("Auto-loading level: {0}", cleanArg);
      OpenScene(cleanArg);
    } else if (ext == ".source") {
      S67_CORE_INFO("Auto-loading project: {0}", cleanArg);
      DiscoverProject(p);

      // Auto-load Default Level if set
      if (!m_ProjectDefaultLevel.empty()) {
        std::filesystem::path defaultLevelPath =
            ResolveAssetPath(m_ProjectDefaultLevel);
        if (std::filesystem::exists(defaultLevelPath)) {
          S67_CORE_INFO("Auto-loading default project level: {0}",
                        defaultLevelPath.string());
          OpenScene(defaultLevelPath.string());
        }
      }
    }
  }
#endif

  S67_CORE_INFO("Application initialized successfully");
}

void Application::InitDefaultAssets() {
  auto vertexArray = VertexArray::Create();

  float vertices[] = {
      // Position           // Normals           // TexCoords
      // Front
      -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,
      0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
      // Back
      -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f,
      0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
      1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
      // Top
      -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f,
      1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
      -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
      // Bottom
      -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f,
      0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f,
      0.0f, 1.0f, -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
      // Left
      -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, -1.0f, 1.0f,
      -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
      1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
      // Right
      1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 1.0f,
      0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
      1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f};

  Ref<VertexBuffer> vertexBuffer =
      VertexBuffer::Create(vertices, sizeof(vertices));
  vertexBuffer->SetLayout({{ShaderDataType::Float3, "a_Position"},
                           {ShaderDataType::Float3, "a_Normal"},
                           {ShaderDataType::Float2, "a_TexCoord"}});
  vertexArray->AddVertexBuffer(vertexBuffer);

  uint32_t indices[] = {0,  1,  2,  2,  3,  0,  4,  5,  6,  6,  7,  4,
                        8,  9,  10, 10, 11, 8,  12, 13, 14, 14, 15, 12,
                        16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20};
  vertexArray->SetIndexBuffer(
      IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));

  m_DefaultShader =
      Shader::Create(ResolveAssetPath("assets/shaders/Lighting.glsl").string());
  m_DefaultTexture = Texture2D::Create(
      ResolveAssetPath("assets/textures/Checkerboard.png").string());
  m_CubeMesh = vertexArray;
}

void Application::CreateTestScene() {
  S67_CORE_INFO("Setting up test scene...");
  auto &bodyInterface = PhysicsSystem::GetBodyInterface();

  // 1. Floor (Anchored)
  auto floor =
      CreateRef<Entity>("Floor", m_CubeMesh, m_DefaultShader, m_DefaultTexture);
  floor->Transform.Position = {0.0f, -2.0f, 0.0f};
  floor->Transform.Scale = {20.0f, 1.0f, 20.0f};
  floor->Anchored = true;

  JPH::BodyCreationSettings floorSettings(
      PhysicsShapes::CreateBox({20.0f, 1.0f, 20.0f}), JPH::RVec3(0, -2, 0),
      JPH::Quat::sIdentity(),
      floor->Anchored ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic,
      floor->Anchored ? Layers::NON_MOVING : Layers::MOVING);
  floorSettings.mUserData = (uint64_t)floor.get();
  floor->PhysicsBody = bodyInterface.CreateAndAddBody(
      floorSettings, JPH::EActivation::DontActivate);
  m_Scene->AddEntity(floor);

  // 2. Dynamic Cubes
  for (int i = 0; i < 5; i++) {
    std::string name = "Cube " + std::to_string(i);
    auto cube =
        CreateRef<Entity>(name, m_CubeMesh, m_DefaultShader, m_DefaultTexture);
    cube->Transform.Position = {(float)i * 2.0f - 4.0f, 10.0f + (float)i * 2.0f,
                                0.0f};
    cube->Anchored = false;

    JPH::BodyCreationSettings cubeSettings(
        PhysicsShapes::CreateBox({1.0f, 1.0f, 1.0f}),
        JPH::RVec3(cube->Transform.Position.x, cube->Transform.Position.y,
                   cube->Transform.Position.z),
        JPH::Quat::sIdentity(),
        cube->Anchored ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic,
        cube->Anchored ? Layers::NON_MOVING : Layers::MOVING);
    cubeSettings.mUserData = (uint64_t)cube.get();
    cube->PhysicsBody = bodyInterface.CreateAndAddBody(
        cubeSettings, JPH::EActivation::Activate);
    m_Scene->AddEntity(cube);
  }
  m_Scene->EnsurePlayerExists();
}

Application::~Application() {
  HUDRenderer::Shutdown();
#ifndef S67_RUNTIME
  m_ImGuiLayer->OnDetach();
#endif
  PhysicsSystem::Shutdown();
}

void Application::OnScenePlay() {
  if (m_ProjectRoot.empty() || !m_LevelLoaded) {
    S67_CORE_WARN("Cannot enter Play Mode: No project or level loaded!");
    return;
  }

  m_Scene->EnsurePlayerExists();

  if (m_SceneState == SceneState::Edit) {
    // Backup before first play
    s_SceneBackup.Data.clear();
    for (auto &entity : m_Scene->GetEntities()) {
      s_SceneBackup.Data[entity.get()] = {entity->Transform.Position,
                                          entity->Transform.Rotation,
                                          entity->Transform.Scale};
    }

    float fov = 45.0f;
    glm::vec3 startPos = {0.0f, 2.0f, 0.0f};
    glm::vec3 startRotation = {0.0f, 0.0f, 0.0f};

    for (auto &entity : m_Scene->GetEntities()) {
      if (entity->Name == "Player") {
        startPos = entity->Transform.Position;
        startRotation = entity->Transform.Rotation;
        fov = entity->CameraFOV;

        if (auto *pc = entity->GetScript<PlayerController>()) {
          pc->Reset(startPos);
          pc->SetRotation(startRotation.y, startRotation.x);
        }
        break;
      }
    }

    float aspect = 1.0f;
#ifndef S67_RUNTIME
    if (m_GameViewportSize.x > 0 && m_GameViewportSize.y > 0)
      aspect = m_GameViewportSize.x / m_GameViewportSize.y;
#else
    if (m_Window->GetHeight() > 0)
        aspect = (float)m_Window->GetWidth() / (float)m_Window->GetHeight();
#endif

    m_Camera->SetProjection(fov, aspect, 0.1f, 100.0f);
  }

  m_Window->SetCursorLocked(true);
  m_CursorLocked = true;
  m_SceneState = SceneState::Play;
}

void Application::OnScenePause() {
  if (m_SceneState != SceneState::Play)
    return;

  m_SceneState = SceneState::Pause;

  // Re-enable ImGui mouse handling
#ifndef S67_RUNTIME
  ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
#endif

  m_Window->SetCursorLocked(false);
  m_CursorLocked = false;
}

void Application::OnSceneStop() {
  m_SceneState = SceneState::Edit;

  // Re-enable ImGui mouse handling
#ifndef S67_RUNTIME
  ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
#endif

  m_Window->SetCursorLocked(false);
  m_CursorLocked = false;

  // Restore
  auto &bodyInterface = PhysicsSystem::GetBodyInterface();
  for (auto &entity : m_Scene->GetEntities()) {
    if (s_SceneBackup.Data.count(entity.get())) {
      auto &data = s_SceneBackup.Data[entity.get()];
      entity->Transform.Position = data.Position;
      entity->Transform.Rotation = data.Rotation;
      entity->Transform.Scale = data.Scale;

      if (!entity->PhysicsBody.IsInvalid()) {
        glm::quat q = glm::quat(glm::radians(entity->Transform.Rotation));
        JPH::Quat jRotate(q.x, q.y, q.z, q.w);
        bodyInterface.SetPositionAndRotation(
            entity->PhysicsBody,
            JPH::RVec3(data.Position.x, data.Position.y, data.Position.z),
            jRotate, JPH::EActivation::DontActivate);
        bodyInterface.SetLinearAndAngularVelocity(
            entity->PhysicsBody, JPH::Vec3::sZero(), JPH::Vec3::sZero());
      }
    }
  }

  // Final sync: Update PlayerController and Camera to the restored state
  for (auto &entity : m_Scene->GetEntities()) {
    if (entity->Name == "Player") {
      if (auto *pc = entity->GetScript<PlayerController>()) {
        pc->Reset(entity->Transform.Position);
        pc->SetRotation(entity->Transform.Rotation.y,
                        entity->Transform.Rotation.x);
      }

      m_Camera->SetPosition(entity->Transform.Position +
                            glm::vec3(0.0f, 1.7f, 0.0f));
      m_Camera->SetYaw(entity->Transform.Rotation.y - 90.0f);
      m_Camera->SetPitch(entity->Transform.Rotation.x);
      break;
    }
  }
}

void Application::SetProjectRoot(const std::filesystem::path &root) {
  m_ProjectRoot = root;
#ifndef S67_RUNTIME
  if (m_ContentBrowserPanel)
    m_ContentBrowserPanel->SetRoot(root);
#endif

  // Unload previous project modules
  ScriptRegistry::Get().UnloadModules();

  // Scan for scripts in the project root
  std::filesystem::path scriptsDir = root / "scripts";
  if (std::filesystem::exists(scriptsDir)) {
    S67_CORE_INFO("Loading project scripts from: {0}", scriptsDir.string());
    ScriptRegistry::Get().LoadModules(scriptsDir);
  }
}

std::filesystem::path
Application::ResolveAssetPath(const std::filesystem::path &path) {
  if (path.is_absolute())
    return path;

  // 1. Try project root
  if (!m_ProjectRoot.empty()) {
    std::filesystem::path projectPath = m_ProjectRoot / path;
    if (std::filesystem::exists(projectPath))
      return projectPath;
  }

  // 2. Try engine assets root
  if (!m_EngineAssetsRoot.empty()) {
    std::filesystem::path enginePath = m_EngineAssetsRoot / path;
    if (std::filesystem::exists(enginePath))
      return enginePath;
  }

  return path;
}

void Application::OnNewProject() {
  std::string path = FileDialogs::SaveFile(
      "Source67 Project (manifest.source)\0manifest.source\0", "manifest",
      "source");
  if (!path.empty()) {
    std::filesystem::path manifestPath(path);
    // Create Project Directories
    std::filesystem::path projectRoot = manifestPath.parent_path();
    m_ProjectName = projectRoot.stem().string();
    m_ProjectCompany = "Untitled Company";
    m_ProjectVersion = "1.0.0";

    std::filesystem::path projectAssets = projectRoot / "assets";
    std::filesystem::path projectScripts =
        projectRoot / "scripts"; // Lowercase as requested
    std::filesystem::create_directories(projectAssets / "shaders");
    std::filesystem::create_directories(projectAssets / "textures");
    std::filesystem::create_directories(projectScripts);

    // Create Player.cpp Template
    {
      std::string playerScript = R"(#include <S67.h>

class Player : public S67::ScriptableEntity {
public:
    void OnCreate() override {
        S67::Console::Get().AddLog("Player Script Created!");
    }

    void OnUpdate(float ts) override {
        // Movement Logic will go here...
        // For now, this is just a template.
    }
};
)";
      std::ofstream out(projectScripts / "Player.cpp");
      out << playerScript;
      out.close();
    }

    // Copy Default Assets (Shaders)

    // Copy Default Assets (Shaders)
    try {
      std::filesystem::path engineShaders =
          m_EngineAssetsRoot / "assets/shaders";
      if (std::filesystem::exists(engineShaders)) {
        for (const auto &entry :
             std::filesystem::directory_iterator(engineShaders)) {
          if (entry.path().extension() == ".glsl") {
            std::filesystem::copy_file(
                entry.path(),
                projectAssets / "shaders" / entry.path().filename(),
                std::filesystem::copy_options::overwrite_existing);
          }
        }
      }

      // Copy Default Assets (Textures)
      std::filesystem::path engineTextures =
          m_EngineAssetsRoot / "assets/textures";
      if (std::filesystem::exists(engineTextures)) {
        for (const auto &entry :
             std::filesystem::directory_iterator(engineTextures)) {
          if (entry.path().extension() == ".png") {
            // Skip UI icons that should stay in the engine assets
            if (entry.path().filename() == "level_icon.png" ||
                entry.path().filename() == "folder_icon.png" ||
                entry.path().filename() == "back_arrow_icon.png" ||
                entry.path().filename() == "engine_logo.png") {
              continue;
            }

            std::filesystem::copy_file(
                entry.path(),
                projectAssets / "textures" / entry.path().filename(),
                std::filesystem::copy_options::overwrite_existing);
          }
        }
      }
    } catch (const std::exception &e) {
      S67_CORE_ERROR("Failed to copy default assets: {0}", e.what());
    }

    SetProjectRoot(projectRoot);
    m_ProjectFilePath = manifestPath;
    SaveManifest();
    AddToRecentProjects(projectRoot.string());
    S67_CORE_INFO("Created new project manifest and isolated assets at: {0}",
                  projectRoot.string());
  }
}

void Application::SaveManifest() {
  if (m_ProjectRoot.empty())
    return;

  std::filesystem::path manifestPath = m_ProjectRoot / "manifest.source";
  nlohmann::json root;
  root["ProjectName"] = m_ProjectName;
  root["Company"] = m_ProjectCompany;
  root["Version"] = m_ProjectVersion;
  root["DefaultLevel"] = m_ProjectDefaultLevel;

  std::ofstream fout(manifestPath);
  if (fout.is_open()) {
    fout << root.dump(2);
    fout.close();
    S67_CORE_INFO("Saved project manifest to {0}", manifestPath.string());
  } else {
    S67_CORE_ERROR("Failed to save project manifest to {0}",
                   manifestPath.string());
  }
}

void Application::OnOpenProject() {
  std::string path = FileDialogs::OpenFolder();
  if (!path.empty()) {
    std::filesystem::path folderPath(path);
    SetProjectRoot(folderPath);

    // Try to find a manifest in this specific folder
    std::filesystem::path manifestPath = folderPath / "manifest.source";
    if (std::filesystem::exists(manifestPath)) {
      // If opening a level later, it will discover properly, but let's load it
      // now too
      DiscoverProject(
          manifestPath); // Discovery takes a "level" path essentially

      // Auto-load Default Level if set
      if (!m_ProjectDefaultLevel.empty()) {
        std::filesystem::path defaultLevelPath =
            ResolveAssetPath(m_ProjectDefaultLevel);
        if (std::filesystem::exists(defaultLevelPath)) {
          S67_CORE_INFO("Auto-loading default party level: {0}",
                        defaultLevelPath.string());
          OpenScene(defaultLevelPath.string());
        }
      }
    } else {
      m_ProjectName = folderPath.stem().string();
      m_ProjectVersion = "Developer Root";
      m_ProjectFilePath = "";
    }
    AddToRecentProjects(folderPath.string());
    S67_CORE_INFO("Opened project folder: {0}", path);
  }
}

void Application::OnBuildRuntime() {
  if (m_ProjectRoot.empty()) {
    S67_CORE_WARN("Please open a project first.");
    return;
  }

  std::string outputDir = FileDialogs::OpenFolder();
  if (outputDir.empty())
    return;

  std::filesystem::path projectRoot = m_ProjectRoot;
  std::filesystem::path outDir = outputDir;
  std::string projectName = m_ProjectName;

  // Run in a separate thread to prevent UI freezing
  std::thread([projectRoot, outDir, projectName]() {
    try {
      S67_CORE_INFO("Starting Export to: {0}", outDir.string());

      // 1. Locate Runtime Executable Template
      std::string runtimeName = "Source67-Runtime";
#ifdef _WIN32
      runtimeName += ".exe";
#endif

      // Look in the current working directory (where Editor is running)
      std::filesystem::path currentPath = std::filesystem::current_path();
      std::filesystem::path runtimePath = currentPath / runtimeName;

      if (!std::filesystem::exists(runtimePath)) {
        S67_CORE_ERROR(
            "Failed to locate Runtime Executable template at: {0}",
            runtimePath.string());
        S67_CORE_ERROR("Ensure Source67-Runtime is built and present next to "
                       "the Editor.");
        return;
      }

      // 2. Prepare Output Directory
      if (!std::filesystem::exists(outDir))
        std::filesystem::create_directories(outDir);

      // 3. Copy Executable -> Output/ProjectName.exe
      std::string targetExeName = projectName;
#ifdef _WIN32
      targetExeName += ".exe";
#endif
      std::filesystem::path targetExePath = outDir / targetExeName;

      std::filesystem::copy_file(runtimePath, targetExePath,
                                 std::filesystem::copy_options::overwrite_existing);
      S67_CORE_INFO("Copied Runtime to {0}", targetExePath.string());

      // 4. Copy Assets
      // A. Engine Defaults (if 'assets' folder is next to Editor)
      std::filesystem::path engineAssets = currentPath / "assets";
      std::filesystem::path targetAssets = outDir / "assets";
      
      if (!std::filesystem::exists(targetAssets))
          std::filesystem::create_directories(targetAssets);

      auto copyOptions = std::filesystem::copy_options::recursive |
                         std::filesystem::copy_options::overwrite_existing;

      if (std::filesystem::exists(engineAssets)) {
        std::filesystem::copy(engineAssets, targetAssets, copyOptions);
        S67_CORE_INFO("Copied Engine Assets.");
      }

      // B. Project Assets (Overwrite Engine defaults)
      std::filesystem::path projectAssets = projectRoot / "assets";
      if (!std::filesystem::exists(projectAssets))
        projectAssets = projectRoot / "Assets";

      if (std::filesystem::exists(projectAssets)) {
        std::filesystem::copy(projectAssets, targetAssets, copyOptions);
        S67_CORE_INFO("Copied Project Assets.");
      }

      // 5. Copy Scripts
      std::filesystem::path projectScripts = projectRoot / "scripts";
      if (!std::filesystem::exists(projectScripts))
        projectScripts = projectRoot / "Scripts";

      if (std::filesystem::exists(projectScripts)) {
        std::filesystem::path targetScripts = outDir / "scripts";
        if (!std::filesystem::exists(targetScripts))
            std::filesystem::create_directories(targetScripts);
            
        std::filesystem::copy(projectScripts, targetScripts, copyOptions);
        S67_CORE_INFO("Copied Scripts.");
      }

      // 6. Copy Manifest
      std::filesystem::path manifestPath = projectRoot / "manifest.source";
      if (std::filesystem::exists(manifestPath)) {
        std::filesystem::copy_file(
            manifestPath, outDir / "manifest.source",
            std::filesystem::copy_options::overwrite_existing);
      }

      S67_CORE_INFO("Export Complete Successfully!");

    } catch (const std::exception &e) {
      S67_CORE_ERROR("Export Failed: {0}", e.what());
    }
  }).detach();
}

void Application::AddToRecentProjects(const std::string &path) {
  if (path.empty())
    return;

  // Move to front if exists, or add to front
  auto it = std::find(m_RecentProjects.begin(), m_RecentProjects.end(), path);
  if (it != m_RecentProjects.end()) {
    m_RecentProjects.erase(it);
  }
  m_RecentProjects.insert(m_RecentProjects.begin(), path);

  // Limit to 5
  if (m_RecentProjects.size() > 5) {
    m_RecentProjects.pop_back();
  }

  SaveSettings();
}

void Application::DiscoverProject(const std::filesystem::path &levelPath) {
  std::filesystem::path currentDir = levelPath.parent_path();
  bool found = false;

  // Search upward for manifest.source
  while (!currentDir.empty() && currentDir != currentDir.root_path()) {
    std::filesystem::path manifestPath = currentDir / "manifest.source";
    if (std::filesystem::exists(manifestPath)) {
      m_ProjectFilePath = manifestPath;
      SetProjectRoot(currentDir);

      // Parse Manifest (JSON)
      try {
        std::ifstream fin(manifestPath);
        nlohmann::json data = nlohmann::json::parse(fin);
        m_ProjectName = data.value("ProjectName", "Unnamed Project");
        m_ProjectCompany = data.value("Company", "Untitled Company");
        m_ProjectVersion = data.value("Version", "1.0.0");
        m_ProjectDefaultLevel = data.value("DefaultLevel", "");

        S67_CORE_INFO("Discovered project: {0} (v{1}) at {2}", m_ProjectName,
                      m_ProjectVersion, currentDir.string());
        AddToRecentProjects(currentDir.string());
        found = true;
        break;
      } catch (const std::exception &e) {
        S67_CORE_ERROR("Failed to parse manifest at {0}: {1}",
                       manifestPath.string(), e.what());
        // Continue searching upward if this one is corrupted
      }
    }
    currentDir = currentDir.parent_path();
  }

  if (!found) {
    m_ProjectName = "Standalone";
    m_ProjectVersion = "N/A";
    m_ProjectFilePath = "";
    // Keep current root or set to assets
  }
}

void Application::OnNewScene() {
  // Check for unsaved changes
  if (m_SceneModified) {
#ifndef S67_RUNTIME
    ImGui::OpenPopup("Unsaved Changes##NewScene");
#endif
    return;
  }

  m_Scene->Clear();
#ifndef S67_RUNTIME
  m_SceneHierarchyPanel->SetSelectedEntity(nullptr);
#endif

  PhysicsSystem::Shutdown();
  PhysicsSystem::Init();

  CreateTestScene();

  m_LevelLoaded = true;
  m_LevelFilePath = "Untitled.s67";
  m_Window->SetCursorLocked(false);
  m_CursorLocked = false;
  m_SceneModified = false;
#ifndef S67_RUNTIME
  ImGui::SetWindowFocus("Scene");
#endif
  S67_CORE_INFO("Created new level");
}

void Application::CloseScene() {
  m_Scene->Clear();
#ifndef S67_RUNTIME
  m_SceneHierarchyPanel->SetSelectedEntity(nullptr);
#endif
  m_LevelLoaded = false;
  m_LevelFilePath = "";
  m_ProjectName = "Standalone";
  m_ProjectVersion = "N/A";
  S67_CORE_INFO("Closed level");
}

void Application::CloseProject() {
  CloseScene();
  m_ProjectRoot = "";
  m_ProjectFilePath = "";
  m_ProjectName = "Standalone";
  m_ProjectVersion = "N/A";
#ifndef S67_RUNTIME
  m_ContentBrowserPanel->SetRoot("");
#endif
  S67_CORE_INFO("Closed project");
}

void Application::OnSaveScene() {
  if (m_SceneState != SceneState::Edit) {
    S67_CORE_WARN("Cannot save while playing!");
    return;
  }

  if (m_LevelLoaded && !m_LevelFilePath.empty() &&
      m_LevelFilePath != "Untitled.s67") {
    SceneSerializer serializer(m_Scene.get(), m_ProjectRoot.string());
    serializer.Serialize(m_LevelFilePath);
    m_SceneModified = false;
    m_ShowSaveNotification = true;
    m_SaveNotificationTime = 0.0f;
    S67_CORE_INFO("Quick Saved level: {0}", m_LevelFilePath);
  } else {
    OnSaveSceneAs();
  }
}

void Application::OnSaveSceneAs() {
  if (m_SceneState != SceneState::Edit) {
    S67_CORE_WARN("Cannot save while playing!");
    return;
  }

  std::string defaultName =
      m_LevelLoaded ? std::filesystem::path(m_LevelFilePath).stem().string()
                    : "level";
  std::string filepath = FileDialogs::SaveFile(
      "Source67 Level (*.s67)\0*.s67\0", defaultName.c_str(), "s67");
  if (!filepath.empty()) {
    SceneSerializer serializer(m_Scene.get(), m_ProjectRoot.string());
    serializer.Serialize(filepath);
    m_LevelLoaded = true;
    m_LevelFilePath = filepath;
    m_SceneModified = false;
    m_ShowSaveNotification = true;
    m_SaveNotificationTime = 0.0f;
    DiscoverProject(std::filesystem::path(filepath));
  }
}

void Application::OnOpenScene() {
  if (m_ProjectRoot.empty()) {
    S67_CORE_WARN("Cannot open level without a project loaded!");
    return;
  }

  if (m_SceneState != SceneState::Edit) {
    S67_CORE_WARN("Cannot load while playing!");
    return;
  }

  std::string filepath =
      FileDialogs::OpenFile("Source67 Level (*.s67)\0*.s67\0", "s67");
  if (!filepath.empty()) {
    // Check for unsaved changes AFTER user selects a file
    if (m_SceneModified) {
      m_PendingScenePath = filepath;
#ifndef S67_RUNTIME
      ImGui::OpenPopup("Unsaved Changes##OpenScene");
#endif
      return;
    }
    OpenScene(filepath);
  }
}

void Application::OpenScene(const std::string &filepath) {
  // Check for unsaved changes
  if (m_SceneModified) {
    m_PendingScenePath = filepath;
#ifndef S67_RUNTIME
    ImGui::OpenPopup("Unsaved Changes##OpenSceneDirect");
#endif
    return;
  }

  PhysicsSystem::Shutdown(); // Reset physics system to clear all bodies
  PhysicsSystem::Init();
  // m_PlayerController is managed by Scene's script system now

  DiscoverProject(std::filesystem::path(filepath));
  SceneSerializer serializer(m_Scene.get(), m_ProjectRoot.string());
  if (serializer.Deserialize(filepath)) {
    m_LevelLoaded = true;
    m_LevelFilePath = filepath;
    m_SceneModified = false;
    m_Window->SetCursorLocked(false);
    m_CursorLocked = false;
#ifndef S67_RUNTIME
    ImGui::SetWindowFocus("Scene");
#endif
    auto &bodyInterface = PhysicsSystem::GetBodyInterface();

    for (auto &entity : m_Scene->GetEntities()) {

      // Assign cube mesh if MeshPath is "Cube"
      if (entity->MeshPath == "Cube") {
        entity->Mesh = m_CubeMesh;
      }

      // Recreate Physics Body (Skip Player and Non-Collidable)
      if (entity->Name == "Player" || !entity->Collidable)
        continue;

      glm::quat q = glm::quat(glm::radians(entity->Transform.Rotation));
      JPH::BodyCreationSettings settings(
          PhysicsShapes::CreateBox({entity->Transform.Scale.x,
                                    entity->Transform.Scale.y,
                                    entity->Transform.Scale.z}),
          JPH::RVec3(entity->Transform.Position.x, entity->Transform.Position.y,
                     entity->Transform.Position.z),
          JPH::Quat(q.x, q.y, q.z, q.w),
          entity->Anchored ? JPH::EMotionType::Static
                           : JPH::EMotionType::Dynamic,
          entity->Anchored ? Layers::NON_MOVING : Layers::MOVING);

      settings.mUserData = (uint64_t)entity.get();
      entity->PhysicsBody =
          bodyInterface.CreateAndAddBody(settings, JPH::EActivation::Activate);
    }
    m_Scene->EnsurePlayerExists();
  }
}

void Application::OnEntityCollidableChanged(Ref<Entity> entity) {
  if (!entity)
    return;

  auto &bodyInterface = PhysicsSystem::GetBodyInterface();
  if (!entity->PhysicsBody.IsInvalid()) {
    bodyInterface.RemoveBody(entity->PhysicsBody);
    bodyInterface.DestroyBody(entity->PhysicsBody);
  }

  if (entity->Collidable) {
    glm::quat q = glm::quat(glm::radians(entity->Transform.Rotation));

    JPH::BodyCreationSettings settings(
        PhysicsShapes::CreateBox({entity->Transform.Scale.x,
                                  entity->Transform.Scale.y,
                                  entity->Transform.Scale.z}),
        JPH::RVec3(entity->Transform.Position.x, entity->Transform.Position.y,
                   entity->Transform.Position.z),
        JPH::Quat(q.x, q.y, q.z, q.w),
        entity->Anchored ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic,
        entity->Anchored ? Layers::NON_MOVING : Layers::MOVING);

    settings.mUserData = (uint64_t)entity.get();

    entity->PhysicsBody =
        bodyInterface.CreateAndAddBody(settings, JPH::EActivation::Activate);
  } else {
    // Ensure the ID is invalidated if we just removed it
    entity->PhysicsBody = JPH::BodyID();
  }
}

void Application::OnEvent(Event &e) {
#ifdef S67_RUNTIME
  EventDispatcher dispatcher(e);
  dispatcher.Dispatch<WindowCloseEvent>(
      BIND_EVENT_FN(Application::OnWindowClose));
  dispatcher.Dispatch<WindowResizeEvent>(
      BIND_EVENT_FN(Application::OnWindowResize));

  // Player / Scripts
  if (m_Scene && m_SceneState == SceneState::Play) {
    if (auto entity = m_Scene->FindEntityByName("Player")) {
      if (auto *pc = entity->GetScript<PlayerController>()) {
        pc->OnEvent(e);
      }
    }
  }
#else
  // 1. Console Toggle (Global Priority)
  if (e.GetEventType() == EventType::KeyPressed) {
    auto &ek = (KeyPressedEvent &)e;
    if (ek.GetKeyCode() == GLFW_KEY_GRAVE_ACCENT && m_EnableConsole) {
      m_ShowConsole = !m_ShowConsole;

      // UX: Handle Cursor & Input Blocking
      if (m_ShowConsole) {
        // Console Opened: Unlock cursor for interaction
        m_Window->SetCursorLocked(false);
        m_CursorLocked = false;

        // Disable rotation if in editor
        if (m_SceneState == SceneState::Edit)
          m_EditorCameraController->SetRotationEnabled(false);

      } else {
        // Console Closed: Restore state
        if (m_SceneState == SceneState::Play) {
          m_Window->SetCursorLocked(true);
          m_CursorLocked = true;
        } else if (m_SceneState == SceneState::Edit) {
          m_Window->SetCursorLocked(false);
          m_CursorLocked = false;
        }
      }
    }
  }

  // 2. Console Input Blocking
  if (m_ShowConsole) {
    m_ImGuiLayer->OnEvent(e);
    return;
  }

  m_ImGuiLayer->OnEvent(e);

  if (m_SceneState == SceneState::Play) {
    if (m_Scene) {
      if (auto entity = m_Scene->FindEntityByName("Player")) {
        if (auto *pc = entity->GetScript<PlayerController>()) {
          pc->OnEvent(e);
        }
      }
    }

    // Global ESC handler for Play Mode
    if (e.GetEventType() == EventType::KeyPressed) {
      auto &ek = (KeyPressedEvent &)e;
      if (ek.GetKeyCode() == GLFW_KEY_ESCAPE)
        OnScenePause();
    }

  } else {
    // Editor Navigation logic
    if (e.GetEventType() == EventType::MouseButtonPressed) {
      auto &mb = (MouseButtonPressedEvent &)e;
      if (mb.GetMouseButton() == 1) { // Right Click
        if (m_SceneViewportHovered) {
          m_Window->SetCursorLocked(true);
          m_CursorLocked = true;
          m_EditorCameraController->SetRotationEnabled(true);
          m_EditorCameraController->SetFirstMouse(true);
        }
      } else if (mb.GetMouseButton() == 0) { // Left Click
        if (m_SceneViewportHovered && m_SceneState != SceneState::Play) {
          if (ImGuizmo::IsOver())
            return;

          // Mouse Picking
          ImVec2 mousePos = ImGui::GetMousePos();
          float x = mousePos.x - m_SceneViewportPos.x;
          float y = mousePos.y - m_SceneViewportPos.y;

          // NDC (-1 to 1)
          float ndcX = (2.0f * x) / m_SceneViewportSize.x - 1.0f;
          float ndcY = 1.0f - (2.0f * y) / m_SceneViewportSize.y;

          glm::mat4 invVP =
              glm::inverse(m_EditorCamera->GetViewProjectionMatrix());
          glm::vec4 ndcRayNear = {ndcX, ndcY, -1.0f, 1.0f};
          glm::vec4 ndcRayFar = {ndcX, ndcY, 1.0f, 1.0f};

          glm::vec4 worldRayNear = invVP * ndcRayNear;
          glm::vec4 worldRayFar = invVP * ndcRayFar;

          worldRayNear /= worldRayNear.w;
          worldRayFar /= worldRayFar.w;

          glm::vec3 rayOrigin = glm::vec3(worldRayNear);
          glm::vec3 rayDirection =
              glm::normalize(glm::vec3(worldRayFar - worldRayNear));

          JPH::BodyID hitID =
              PhysicsSystem::Raycast(rayOrigin, rayDirection, 1000.0f);
          if (!hitID.IsInvalid()) {
            for (auto &entity : m_Scene->GetEntities()) {
              if (entity->PhysicsBody == hitID) {
                m_SceneHierarchyPanel->SetSelectedEntity(entity);
                break;
              }
            }
          } else {
            m_SceneHierarchyPanel->SetSelectedEntity(nullptr);
          }
        }
      }

      m_EditorCameraController->OnEvent(e);
    }

    // Handle mouse button RELEASE
    if (e.GetEventType() == EventType::MouseButtonReleased) {
      auto &mb = (MouseButtonReleasedEvent &)e;
      if (mb.GetMouseButton() == 1) { // Right Click
        m_Window->SetCursorLocked(false);
        m_CursorLocked = false;
        m_EditorCameraController->SetRotationEnabled(false);
      }
    }

    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>(
        BIND_EVENT_FN(Application::OnWindowClose));
    dispatcher.Dispatch<WindowResizeEvent>(
        BIND_EVENT_FN(Application::OnWindowResize));
    dispatcher.Dispatch<WindowDropEvent>(
        BIND_EVENT_FN(Application::OnWindowDrop));

    // Editor Shortcuts
    if (e.GetEventType() == EventType::KeyPressed) {
      auto &ek = (KeyPressedEvent &)e;
      bool control = Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL) ||
                     Input::IsKeyPressed(GLFW_KEY_RIGHT_CONTROL);
      bool super = Input::IsKeyPressed(GLFW_KEY_LEFT_SUPER) ||
                   Input::IsKeyPressed(GLFW_KEY_RIGHT_SUPER);

      if (ek.GetKeyCode() == GLFW_KEY_S && (control || super)) {
        OnSaveScene();
        m_ShowSaveNotification = true;
        m_SaveNotificationTime = (float)glfwGetTime();
      }

      // Undo/Redo
      if (control || super) {
        if (ek.GetKeyCode() == GLFW_KEY_Z) {
          if (Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT) ||
              Input::IsKeyPressed(GLFW_KEY_RIGHT_SHIFT))
            m_UndoSystem.Redo();
          else
            m_UndoSystem.Undo();
        }
        if (ek.GetKeyCode() == GLFW_KEY_Y) {
          m_UndoSystem.Redo();
        }
      }

      // Gizmo shortcuts
      if (!m_EditorCameraController->IsRotationEnabled()) {
        if (ek.GetKeyCode() == GLFW_KEY_Q)
          m_GizmoType = -1;
        if (ek.GetKeyCode() == GLFW_KEY_W)
          m_GizmoType = (int)ImGuizmo::OPERATION::TRANSLATE;
        if (ek.GetKeyCode() == GLFW_KEY_E)
          m_GizmoType = (int)ImGuizmo::OPERATION::ROTATE;
        if (ek.GetKeyCode() == GLFW_KEY_R)
          m_GizmoType = (int)ImGuizmo::OPERATION::SCALE;
      }

      if (ek.GetKeyCode() == GLFW_KEY_F) {
        auto selectedEntity = m_SceneHierarchyPanel->GetSelectedEntity();
        if (selectedEntity) {
          glm::vec3 pos = selectedEntity->Transform.Position;
          glm::vec3 scale = selectedEntity->Transform.Scale;
          float maxScale = std::max({scale.x, scale.y, scale.z});

          // Move camera to look at object
          glm::vec3 offset = {0.0f, maxScale * 2.0f, maxScale * 5.0f};
          m_EditorCamera->SetPosition(pos + offset);
        }
      }
    }
  }

  // Global ESC handler (fallback)
  if (e.GetEventType() == EventType::KeyPressed) {
    auto &ek = (KeyPressedEvent &)e;
    if (ek.GetKeyCode() == GLFW_KEY_ESCAPE) {
      if (m_SceneState == SceneState::Play) {
        OnScenePause();
      }
      // In other modes we might just want to ensure cursor is unlocked if user
      // jams ESC
      else {
        m_Window->SetCursorLocked(false);
        m_CursorLocked = false;
      }
    }
  }
#endif
}

void Application::Run() {
  m_PreviousFrameTime = glfwGetTime();

  while (m_Running) {
    // PHASE 1: Measure frame time
    double current_frame_time = glfwGetTime();
    double frame_time = current_frame_time - m_PreviousFrameTime;
    m_PreviousFrameTime = current_frame_time;

    // Calculate Game FPS from actual frame time
    m_GameFPS =
        (frame_time > 0.0) ? static_cast<float>(1.0 / frame_time) : 0.0f;

    // PHASE 2: Prevent spiral of death (lag spike safety)
    if (frame_time > MAX_FRAME_TIME) {
      frame_time = MAX_FRAME_TIME; // Clamp to max 250ms
    }

    // PHASE 3: Accumulate real time
    m_Accumulator += frame_time;

    // PHASE 4: Process all due physics ticks
    int tick_count = 0;
    while (m_Accumulator >= m_TickDuration) {
      // Save previous state for interpolation
      m_PreviousState = m_CurrentState;

      // Run one physics tick with fixed delta time
      UpdateGameTick(m_TickDuration);

      // Deduct from accumulator
      m_Accumulator -= m_TickDuration;
      tick_count++;
      m_TickNumber++;
    }

    // PHASE 5: Render frame with interpolation
    float alpha = static_cast<float>(m_Accumulator / m_TickDuration);
    RenderFrame(alpha);

    // PHASE 6: Window update (swap buffers, poll events)
    m_Window->OnUpdate();

    // PHASE 7: Apply FPS cap if enabled (High-precision hybrid wait)
    if (m_FPSCap > 0) {
      double target_frame_time = 1.0 / m_FPSCap;
      double frame_end_time = glfwGetTime();
      double elapsed = frame_end_time - current_frame_time;

      if (elapsed < target_frame_time) {
        double wait_time = target_frame_time - elapsed;

        // 1. Precise sleep: Only sleep if we have more than a safe margin
        // (20ms) to wait. This is necessary because the default Windows
        // scheduler resolution is ~15.6ms. For anything shorter (like 144Hz or
        // 240Hz targets), we MUST busy-wait to stay precise and avoid the "64
        // FPS cap" issue.
        if (wait_time > 0.020) {
          std::this_thread::sleep_for(
              std::chrono::duration<double>(wait_time - 0.018));
        }

        // 2. High-precision busy wait: Spin until we hit the exact microsecond
        // target. This is extremely precise and works for all frame rates.
        while (glfwGetTime() - current_frame_time < target_frame_time) {
          // busy wait
        }
      }
    }
  }
}

void Application::SetTickRate(float rate) {
  if (rate <= 0.0f) return;
  m_TickRate = rate;
  m_TickDuration = 1.0f / rate;
}

void Application::UpdateGameTick(float tick_dt) {
  // This function runs at exactly 66 Hz with fixed TICK_DURATION
  // CRITICAL: Always use tick_dt, NEVER variable frame delta time

  if (m_SceneState != SceneState::Play) {
    // Only run physics ticks during play mode
    return;
  }

  // 1. Update Scene (includes Scripts -> PlayerController::On
  if (m_Scene)
    m_Scene->OnUpdate(tick_dt);

  // 2. Update Jolt Physics with fixed timestep
  PhysicsSystem::OnUpdate(Timestep(tick_dt));

  // 3. Update game state from player controller for interpolation
  if (m_Scene) {
    Ref<Entity> playerEntity = m_Scene->FindEntityByName("Player");
    if (playerEntity) {
      if (auto *pc = playerEntity->GetScript<PlayerController>()) {
        m_CurrentState.player_position =
            m_Camera->GetPosition() - glm::vec3(0.0f, 1.7f, 0.0f);
        m_CurrentState.player_velocity = pc->GetVelocity();
        m_CurrentState.yaw = pc->GetYaw();
        m_CurrentState.pitch = pc->GetPitch();
      }
    }
  }

  // Note: Additional movement state (sprinting, crouching, etc.) could be
  // synchronized here if exposed by PlayerController in the future
}

bool Application::OnWindowClose(WindowCloseEvent & /*e*/) {
  m_Running = false;
  return true;
}

bool Application::OnWindowResize(WindowResizeEvent &e) {
  if (e.GetWidth() == 0 || e.GetHeight() == 0) {
    return false;
  }

  // Realtime resizing support
  // Render with alpha=1.0 (current state, no interpolation during resize)
  // Ensure we manually swap buffers because the main loop is blocked
  RenderFrame(1.0f);
  glfwSwapBuffers((GLFWwindow *)m_Window->GetNativeWindow());

  return false;
}

bool Application::OnWindowDrop(WindowDropEvent &e) {
#ifndef S67_RUNTIME
  if (!m_LevelLoaded) {
    S67_CORE_WARN("Cannot import files without an open project/level!");
    return false;
  }

  std::filesystem::path targetDir =
      m_ContentBrowserPanel->GetCurrentDirectory();

  for (const auto &pathStr : e.GetPaths()) {
    std::filesystem::path sourcePath(pathStr);
    std::filesystem::path targetPath = targetDir / sourcePath.filename();

    try {
      if (std::filesystem::exists(targetPath)) {
        S67_CORE_WARN("File already exists: {0}. Skipping.",
                      targetPath.string());
        continue;
      }

      if (std::filesystem::is_directory(sourcePath)) {
        std::filesystem::copy(sourcePath, targetPath,
                              std::filesystem::copy_options::recursive);
      } else {
        std::filesystem::copy_file(sourcePath, targetPath);
      }
      S67_CORE_INFO("Imported: {0} -> {1}", sourcePath.string(),
                    targetPath.string());
    } catch (const std::filesystem::filesystem_error &err) {
      S67_CORE_ERROR("Failed to import {0}: {1}", sourcePath.string(),
                     err.what());
    }
  }
#endif

  return false;
}

#ifndef S67_RUNTIME
void Application::UI_SettingsWindow() {
  ImGui::SetNextWindowSizeConstraints(ImVec2(600, 450),
                                      ImVec2(FLT_MAX, FLT_MAX));
  if (!ImGui::Begin("Settings", &m_ShowSettingsWindow)) {
    ImGui::End();
    return;
  }

  static int s_SelectedIdx = 0;

  // Left Side: Navigation
  ImGui::BeginChild("SettingsNav", ImVec2(150, 0), true);
  if (ImGui::Selectable("General", s_SelectedIdx == 0))
    s_SelectedIdx = 0;
  if (ImGui::Selectable("Performance", s_SelectedIdx == 1))
    s_SelectedIdx = 1;
  ImGui::EndChild();

  ImGui::SameLine();

  // Right Side: Content
  ImGui::BeginGroup();
  ImGui::BeginChild("SettingsContent",
                    ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

  if (s_SelectedIdx == 0) // General
  {
    if (ImGui::BeginTable("SettingsTable", 2, ImGuiTableFlags_SizingFixedFit)) {
      ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed,
                              150.0f);
      ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

      // Font Scale
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("Font Scale");
      ImGui::TableSetColumnIndex(1);
      ImGui::PushItemWidth(-1.0f);
      if (ImGui::DragFloat("##FontScale", &m_FontSize, 0.01f, 0.5f, 2.0f,
                           "%.2f")) {
        ImGui::GetIO().FontGlobalScale = m_FontSize / 18.0f;
      }
      ImGui::PopItemWidth();

      // Theme
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("Editor Theme");
      ImGui::TableSetColumnIndex(1);
      const char *themes[] = {"Unity Dark", "Dracula", "Classic Dark", "Light"};
      int currentTheme = (int)m_EditorTheme;
      ImGui::PushItemWidth(-1.0f);
      if (ImGui::Combo("##Theme", &currentTheme, themes,
                       IM_ARRAYSIZE(themes))) {
        m_EditorTheme = (EditorTheme)currentTheme;
        switch (m_EditorTheme) {
        case EditorTheme::Unity:
          m_ImGuiLayer->SetDarkThemeColors();
          break;
        case EditorTheme::Dracula:
          m_ImGuiLayer->SetDraculaThemeColors();
          break;
        case EditorTheme::Classic:
          ImGui::StyleColorsDark();
          break;
        case EditorTheme::Light:
          ImGui::StyleColorsLight();
          break;
        }
      }
      ImGui::PopItemWidth();

      // Editor FOV
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("Editor FOV");
      ImGui::TableSetColumnIndex(1);
      ImGui::PushItemWidth(-1.0f);
      if (ImGui::DragFloat("##EditorFOV", &m_EditorFOV, 1.0f, 30.0f, 110.0f,
                           "%.1f")) {
        float aspect = 1.0f;
        if (m_SceneViewportSize.y > 0)
          aspect = m_SceneViewportSize.x / m_SceneViewportSize.y;
        m_EditorCamera->SetProjection(m_EditorFOV, aspect, 0.1f, 1000.0f);
      }
      ImGui::PopItemWidth();

      // Window Background Color
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("Window BG");
      ImGui::TableSetColumnIndex(1);
      if (ImGui::ColorEdit4("##WindowBG", glm::value_ptr(m_CustomColor),
                            ImGuiColorEditFlags_NoInputs)) {
        auto &colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_WindowBg] = ImVec4{m_CustomColor.r, m_CustomColor.g,
                                           m_CustomColor.b, m_CustomColor.a};
      }

      ImGui::EndTable();
    }
  } else if (s_SelectedIdx == 1) // Performance
  {
    if (ImGui::BeginTable("PerformanceTable", 2,
                          ImGuiTableFlags_SizingFixedFit)) {
      ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed,
                              150.0f);
      ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

      // FPS Cap
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("FPS Cap");
      ImGui::TableSetColumnIndex(1);
      ImGui::PushItemWidth(-1.0f);
      ImGui::DragInt("##FPSCap", &m_FPSCap, 1.0f, 0, 1000,
                     m_FPSCap == 0 ? "Unlimited" : "%d");
      ImGui::PopItemWidth();

      // VSync
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("VSync");
      ImGui::TableSetColumnIndex(1);
      if (ImGui::Checkbox("##VSync", &m_VSync)) {
        m_Window->SetVSync(m_VSync);
      }

      ImGui::EndTable();
    }
  }

  ImGui::EndChild();
  ImGui::EndGroup();

  ImGui::End();
}
#endif

#ifndef S67_RUNTIME
void Application::UI_LauncherScreen() {
  ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->Pos);
  ImGui::SetNextWindowSize(viewport->Size);
  ImGui::SetNextWindowViewport(viewport->ID);

  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

  ImGui::Begin("Launcher", nullptr, window_flags);

  // Center Content
  ImVec2 contentSize = {400, 500};
  ImVec2 viewportSize = viewport->Size;
  ImGui::SetCursorPos({(viewportSize.x - contentSize.x) * 0.5f,
                       (viewportSize.y - contentSize.y) * 0.5f});

  ImGui::BeginChild("LauncherContent", contentSize, false);

  // Logo
  if (m_LauncherLogo) {
    float logoWidth = contentSize.x * 0.8f;
    float aspect =
        (float)m_LauncherLogo->GetHeight() / (float)m_LauncherLogo->GetWidth();
    float logoHeight = logoWidth * aspect;

    ImGui::SetCursorPosX((contentSize.x - logoWidth) * 0.5f);
    ImGui::Image((void *)(uint64_t)m_LauncherLogo->GetRendererID(),
                 {logoWidth, logoHeight}, {0, 1}, {1, 0});
  } else {
    // Fallback to title
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::SetWindowFontScale(3.0f);
    const char *title = "Source 67";
    ImVec2 titleSize = ImGui::CalcTextSize(title);
    ImGui::SetCursorPosX((contentSize.x - titleSize.x) * 0.5f);
    ImGui::Text("%s", title);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopFont();
  }

  ImGui::Dummy({0, 40});

  // Buttons
  if (ImGui::Button("New Project", {contentSize.x, 40})) {
    OnNewProject();
  }
  if (ImGui::Button("Open Project", {contentSize.x, 40})) {
    OnOpenProject();
  }

  ImGui::Dummy({0, 20});
  ImGui::Separator();
  ImGui::Dummy({0, 10});

  ImGui::TextDisabled("Recent Projects");
  ImGui::Dummy({0, 5});

  if (m_RecentProjects.empty()) {
    ImGui::TextWrapped("No recent projects found.");
  } else {
    for (const auto &projectPath : m_RecentProjects) {
      if (projectPath.empty())
        continue;

      ImGui::PushID(projectPath.c_str());
      std::filesystem::path p(projectPath);
      std::string label = p.stem().string() + " (" + projectPath + ")";
      if (ImGui::Selectable(label.c_str(), false, 0, {contentSize.x, 0})) {
        SetProjectRoot(p);
        DiscoverProject(p / "manifest.source");
        AddToRecentProjects(projectPath);

        // Auto-load Default Level if set
        if (!m_ProjectDefaultLevel.empty()) {
          std::filesystem::path defaultLevelPath =
              ResolveAssetPath(m_ProjectDefaultLevel);
          if (std::filesystem::exists(defaultLevelPath)) {
            S67_CORE_INFO("Auto-loading default project level: {0}",
                          defaultLevelPath.string());
            OpenScene(defaultLevelPath.string());
          }
        }
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", projectPath.c_str());
      }
      ImGui::PopID();
    }
  }

  // Footer
  ImGui::SetCursorPosY(contentSize.y - 30.0f);
  ImGui::Separator();
  const char *footerText = "Made with   by JsemOlik";
  ImVec2 footerSize = ImGui::CalcTextSize(footerText);
  ImGui::SetCursorPosX((contentSize.x - footerSize.x) * 0.5f);
  ImGui::Text("Made with");
  ImGui::SameLine();
  ImGui::TextColored({1.0f, 0.0f, 0.0f, 1.0f}, "<3");
  ImGui::SameLine();
  ImGui::Text("by");
  ImGui::SameLine();
  ImGui::PushStyleColor(ImGuiCol_Text, {0.3f, 0.6f, 1.0f, 1.0f});
  if (ImGui::Selectable("JsemOlik", false, 0,
                        ImGui::CalcTextSize("JsemOlik"))) {
    FileDialogs::OpenExternally("https://github.com/jsemolik/source67");
  }
  ImGui::PopStyleColor();

  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("https://github.com/jsemolik/source67");
  }

  ImGui::EndChild();
  ImGui::End();
}
#endif

#ifndef S67_RUNTIME
void Application::UI_ProjectSettingsWindow() {
  ImGui::SetNextWindowSizeConstraints(ImVec2(600, 400),
                                      ImVec2(FLT_MAX, FLT_MAX));
  if (!ImGui::Begin("Project Settings", &m_ShowProjectSettingsWindow)) {
    ImGui::End();
    return;
  }

  static int s_ProjSelectedIdx = 0;
  const char *categories[] = {"General", "Paths"};

  ImGui::BeginChild("ProjSidebar", ImVec2(150, 0), true);
  for (int i = 0; i < 2; i++) {
    if (ImGui::Selectable(categories[i], s_ProjSelectedIdx == i)) {
      s_ProjSelectedIdx = i;
    }
  }
  ImGui::EndChild();

  ImGui::SameLine();

  ImGui::BeginChild("ProjContent", ImVec2(0, 0), false);
  ImGui::TextDisabled("%s", categories[s_ProjSelectedIdx]);
  ImGui::Separator();
  ImGui::Dummy(ImVec2(0, 10));

  if (s_ProjSelectedIdx == 0) // General
  {
    if (ImGui::BeginTable("ProjectGeneralTable", 2,
                          ImGuiTableFlags_SizingFixedFit)) {
      ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed,
                              150.0f);
      ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

      // Project Name
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("Project Name");
      ImGui::TableSetColumnIndex(1);
      char nameBuffer[256];
      memset(nameBuffer, 0, sizeof(nameBuffer));
      strncpy(nameBuffer, m_ProjectName.c_str(), sizeof(nameBuffer) - 1);
      ImGui::PushItemWidth(-1.0f);
      if (ImGui::InputText("##ProjectName", nameBuffer, sizeof(nameBuffer))) {
        m_ProjectName = nameBuffer;
      }
      ImGui::PopItemWidth();

      // Version
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("Version");
      ImGui::TableSetColumnIndex(1);
      char versionBuffer[256];
      memset(versionBuffer, 0, sizeof(versionBuffer));
      strncpy(versionBuffer, m_ProjectVersion.c_str(),
              sizeof(versionBuffer) - 1);
      ImGui::PushItemWidth(-1.0f);
      if (ImGui::InputText("##Version", versionBuffer, sizeof(versionBuffer))) {
        m_ProjectVersion = versionBuffer;
      }
      ImGui::PopItemWidth();

      // Company Name
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("Company Name");
      ImGui::TableSetColumnIndex(1);
      char companyBuffer[256];
      memset(companyBuffer, 0, sizeof(companyBuffer));
      strncpy(companyBuffer, m_ProjectCompany.c_str(),
              sizeof(companyBuffer) - 1);
      ImGui::PushItemWidth(-1.0f);
      if (ImGui::InputText("##CompanyName", companyBuffer,
                           sizeof(companyBuffer))) {
        m_ProjectCompany = companyBuffer;
      }
      ImGui::PopItemWidth();

      // Default Level (Dropdown)
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("Default Level");
      ImGui::TableSetColumnIndex(1);

      std::vector<std::string> levelFiles;
      levelFiles.push_back("<None>");
      if (!m_ProjectRoot.empty()) {
        for (const auto &entry :
             std::filesystem::directory_iterator(m_ProjectRoot)) {
          if (entry.path().extension() == ".s67") {
            levelFiles.push_back(entry.path().filename().string());
          }
        }
      }

      const std::string &previewValue =
          m_ProjectDefaultLevel.empty() ? "<None>" : m_ProjectDefaultLevel;

      ImGui::PushItemWidth(-1.0f);
      if (ImGui::BeginCombo("##DefaultLevel", previewValue.c_str())) {
        for (const auto &file : levelFiles) {
          bool isSelected = (previewValue == file);
          if (ImGui::Selectable(file.c_str(), isSelected)) {
            if (file == "<None>")
              m_ProjectDefaultLevel = "";
            else
              m_ProjectDefaultLevel = file;
          }
          if (isSelected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      ImGui::PopItemWidth();

      ImGui::EndTable();
    }
  } else if (s_ProjSelectedIdx == 1) // Paths
  {
    ImGui::Text("Project Root:");
    ImGui::TextDisabled("%s", m_ProjectRoot.string().c_str());
    ImGui::Spacing();
    ImGui::Text("Engine Assets:");
    ImGui::TextDisabled("%s", m_EngineAssetsRoot.string().c_str());
  }

  ImGui::Spacing();
  ImGui::Separator();
  ImVec2 buttonSize = {120, 30};
  ImGui::SetCursorPosY(ImGui::GetWindowHeight() - buttonSize.y -
                       ImGui::GetStyle().WindowPadding.y * 2.0f);
  ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - buttonSize.x);
  if (ImGui::Button("Apply & Save", buttonSize)) {
    SaveManifest();
  }

  ImGui::EndChild();
  ImGui::End();
}
#endif

void Application::SaveSettings() {
  nlohmann::json j;
#ifndef S67_RUNTIME
  j["FontSize"] = m_FontSize;
  j["EditorFOV"] = m_EditorFOV;
#endif
  j["FPSCap"] = m_FPSCap;
  j["VSync"] = m_VSync;
#ifndef S67_RUNTIME
  j["Theme"] = (int)m_EditorTheme;
  j["CustomColor"] = {m_CustomColor.r, m_CustomColor.g, m_CustomColor.b,
                      m_CustomColor.a};

  j["ShowInspector"] = m_ShowInspector;
  j["ShowHierarchy"] = m_ShowHierarchy;
  j["ShowContentBrowser"] = m_ShowContentBrowser;
  j["ShowScene"] = m_ShowScene;
  j["ShowGame"] = m_ShowGame;
  j["ShowToolbar"] = m_ShowToolbar;
  j["ShowStats"] = m_ShowStats;
  j["EnableConsole"] = m_EnableConsole;
#endif

  j["RecentProjects"] = m_RecentProjects;

  std::ofstream o("settings.json");
  o << std::setw(4) << j << std::endl;
  S67_CORE_INFO("Saved settings to settings.json");
}

void Application::LoadSettings() {
  std::ifstream i("settings.json");
  if (i.is_open()) {
    try {
      nlohmann::json j;
      i >> j;
#ifndef S67_RUNTIME
      m_FontSize = j.at("FontSize").get<float>();
      if (j.contains("EditorFOV"))
        m_EditorFOV = j["EditorFOV"];
#endif
      if (j.contains("FPSCap"))
        m_FPSCap = j["FPSCap"];
      if (j.contains("VSync"))
        m_VSync = j["VSync"];

#ifndef S67_RUNTIME
      m_EditorTheme = (EditorTheme)j.at("Theme").get<int>();

      if (j.contains("CustomColor")) {
        m_CustomColor.r = j["CustomColor"][0];
        m_CustomColor.g = j["CustomColor"][1];
        m_CustomColor.b = j["CustomColor"][2];
        m_CustomColor.a = j["CustomColor"][3];
      }

      if (j.contains("ShowInspector"))
        m_ShowInspector = j["ShowInspector"];
      if (j.contains("ShowHierarchy"))
        m_ShowHierarchy = j["ShowHierarchy"];
      if (j.contains("ShowContentBrowser"))
        m_ShowContentBrowser = j["ShowContentBrowser"];
      if (j.contains("ShowScene"))
        m_ShowScene = j["ShowScene"];
      if (j.contains("ShowGame"))
        m_ShowGame = j["ShowGame"];
      if (j.contains("ShowToolbar"))
        m_ShowToolbar = j["ShowToolbar"];
      if (j.contains("ShowStats"))
        m_ShowStats = j["ShowStats"];
      if (j.contains("EnableConsole"))
        m_EnableConsole = j["EnableConsole"];
#endif

      if (j.contains("RecentProjects")) {
        m_RecentProjects = j["RecentProjects"].get<std::vector<std::string>>();
        m_RecentProjects.erase(
            std::remove(m_RecentProjects.begin(), m_RecentProjects.end(), ""),
            m_RecentProjects.end());
      }

#ifndef S67_RUNTIME
      // Apply
      ImGui::GetIO().FontGlobalScale = m_FontSize / 18.0f;
      switch (m_EditorTheme) {
      case EditorTheme::Unity:
        m_ImGuiLayer->SetDarkThemeColors();
        break;
      case EditorTheme::Dracula:
        m_ImGuiLayer->SetDraculaThemeColors();
        break;
      case EditorTheme::Classic:
        ImGui::StyleColorsDark();
        break;
      case EditorTheme::Light:
        ImGui::StyleColorsLight();
        break;
      }

      auto &colors = ImGui::GetStyle().Colors;
      colors[ImGuiCol_WindowBg] = ImVec4{m_CustomColor.r, m_CustomColor.g,
                                         m_CustomColor.b, m_CustomColor.a};
#endif

      if (m_Window)
        m_Window->SetVSync(m_VSync);

      S67_CORE_INFO("Loaded settings from settings.json");
    } catch (...) {
      S67_CORE_ERROR("Error parsing settings.json! Using defaults.");
    }
  } else {
    // Defaults
#ifndef S67_RUNTIME
    m_FontSize = 18.0f;
    m_EditorTheme = EditorTheme::Unity;
    m_ImGuiLayer->SetDarkThemeColors();
    S67_CORE_INFO("No settings.json found, using defaults (Unity Dark, 18px)");
#else
    S67_CORE_INFO("No settings.json found, using defaults");
#endif
  }

  // Apply VSync
  if (m_Window)
    m_Window->SetVSync(m_VSync);

#ifndef S67_RUNTIME
  // Apply editor FOV to the camera if it exists
  if (m_EditorCamera) {
    float aspect = 1280.0f / 720.0f; // Default aspect ratio
    m_EditorCamera->SetProjection(m_EditorFOV, aspect, 0.1f, 1000.0f);
  }
#endif
}

void Application::SaveLayout() {
#ifndef S67_RUNTIME
  if (m_ImGuiLayer)
    m_ImGuiLayer->SaveLayout();
#endif
}

void Application::LoadLayout() {
#ifndef S67_RUNTIME
  if (m_ImGuiLayer)
    m_ImGuiLayer->LoadLayout();
#endif
}

void Application::SaveLayout(const std::string &path) {
#ifndef S67_RUNTIME
  if (m_ImGuiLayer)
    m_ImGuiLayer->SaveLayout(path);
#endif
}

void Application::LoadLayout(const std::string &path) {
#ifndef S67_RUNTIME
  if (m_ImGuiLayer)
    m_ImGuiLayer->LoadLayout(path);
#endif
}

void Application::ResetLayout() {
#ifndef S67_RUNTIME
  m_ShowInspector = true;
  m_ShowHierarchy = true;
  m_ShowContentBrowser = true;
  m_ShowScene = true;
  m_ShowGame = true;
  m_ShowToolbar = true;
  m_ShowStats = true;
  m_ResetLayoutOnNextFrame = true;

  ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
  ImGui::DockBuilderRemoveNode(dockspace_id);
  ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
  ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

  ImGuiID dock_main_id = dockspace_id;
  ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(
      dock_main_id, ImGuiDir_Right, 0.2f, nullptr, &dock_main_id);
  ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(
      dock_main_id, ImGuiDir_Down, 0.3f, nullptr, &dock_main_id);
  ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(
      dock_main_id, ImGuiDir_Left, 0.2f, nullptr, &dock_main_id);
  ImGuiID dock_id_center_bottom = ImGui::DockBuilderSplitNode(
      dock_main_id, ImGuiDir_Down, 0.5f, nullptr, &dock_main_id);

  ImGui::DockBuilderDockWindow("Inspector", dock_id_right);
  ImGui::DockBuilderDockWindow("Scene Hierarchy", dock_id_left);
  ImGui::DockBuilderDockWindow("Content Browser", dock_id_bottom);
  ImGui::DockBuilderDockWindow("Scene", dock_main_id);
  ImGui::DockBuilderDockWindow("Game", dock_id_center_bottom);
  ImGui::DockBuilderDockWindow(
      "Toolbar",
      dock_id_bottom); // Place with content browser or elsewhere
  ImGui::DockBuilderDockWindow("Engine Statistics", dock_id_bottom);

  ImGui::DockBuilderFinish(dockspace_id);
  S67_CORE_INFO("Reset window layout to default");
#endif
}

void Application::RenderFrame(float alpha) {
#ifdef S67_RUNTIME
  Window &window = GetWindow();
  uint32_t width = window.GetWidth();
  uint32_t height = window.GetHeight();
  if (width == 0 || height == 0)
    return;
  glViewport(0, 0, width, height);

  // Camera Update
  glm::vec3 interpolated_position = glm::mix(
      m_PreviousState.player_position, m_CurrentState.player_position, alpha);
  float interpolated_yaw =
      glm::mix(m_PreviousState.yaw, m_CurrentState.yaw, alpha);
  float interpolated_pitch =
      glm::mix(m_PreviousState.pitch, m_CurrentState.pitch, alpha);
  m_Camera->SetPosition(interpolated_position + glm::vec3(0.0f, 1.7f, 0.0f));
  m_Camera->SetYaw(interpolated_yaw);
  m_Camera->SetPitch(interpolated_pitch);
  float aspect = (float)width / (float)height;
  m_Camera->SetProjection(45.0f, aspect, 0.1f, 100.0f);

  // Render Scene
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  Renderer::BeginScene(*m_Camera, m_Sun);
  m_Skybox->Draw(*m_Camera);

  auto &bodyInterface = PhysicsSystem::GetBodyInterface();
  for (auto &entity : m_Scene->GetEntities()) {
    if (entity->Name == "Player")
      continue;
    if (!entity->PhysicsBody.IsInvalid()) {
      JPH::RVec3 position;
      JPH::Quat rotation;
      bodyInterface.GetPositionAndRotation(entity->PhysicsBody, position,
                                           rotation);
      entity->Transform.Position = {position.GetX(), position.GetY(),
                                    position.GetZ()};
      glm::quat q = {rotation.GetW(), rotation.GetX(), rotation.GetY(),
                     rotation.GetZ()};
      entity->Transform.Rotation = glm::degrees(glm::eulerAngles(q));
    }
    if (entity->Material.AlbedoMap)
      entity->Material.AlbedoMap->Bind();
    if (entity->Mesh && entity->MaterialShader &&
        entity->MaterialShader->IsValid()) {
      Renderer::Submit(entity->MaterialShader, entity->Mesh,
                       entity->Transform.GetTransform(),
                       entity->Material.Tiling);
    }
  }
  Renderer::EndScene();

  // HUD
  HUDRenderer::BeginHUD((float)width, (float)height);
  HUDRenderer::RenderCrosshair();
  if (m_Scene) {
    if (auto entity = m_Scene->FindEntityByName("Player")) {
      if (auto *pc = entity->GetScript<PlayerController>()) {
        HUDRenderer::RenderSpeed(pc->GetSpeed() * 52.4934f);
      }
    }
  }
  HUDRenderer::EndHUD();
#else
  // alpha is the interpolation factor between previous and current physics
  // states 0.0 = at previous tick state 0.5 = halfway between previous and
  // current tick ~0.999 = almost at current tick

  // Viewport Resize
  if (FramebufferSpecification spec = m_SceneFramebuffer->GetSpecification();
      m_SceneViewportSize.x > 0.0f && m_SceneViewportSize.y > 0.0f &&
      (spec.Width != (uint32_t)m_SceneViewportSize.x ||
       spec.Height != (uint32_t)m_SceneViewportSize.y)) {
    m_SceneFramebuffer->Resize((uint32_t)m_SceneViewportSize.x,
                               (uint32_t)m_SceneViewportSize.y);
    m_EditorCamera->SetProjection(m_EditorFOV,
                                  m_SceneViewportSize.x / m_SceneViewportSize.y,
                                  0.1f, 100.0f);
  }
  if (FramebufferSpecification spec = m_GameFramebuffer->GetSpecification();
      m_GameViewportSize.x > 0.0f && m_GameViewportSize.y > 0.0f &&
      (spec.Width != (uint32_t)m_GameViewportSize.x ||
       spec.Height != (uint32_t)m_GameViewportSize.y)) {
    m_GameFramebuffer->Resize((uint32_t)m_GameViewportSize.x,
                              (uint32_t)m_GameViewportSize.y);
    m_Camera->SetProjection(45.0f, m_GameViewportSize.x / m_GameViewportSize.y,
                            0.1f, 100.0f);
  }

  // Editor camera updates (still uses per-frame delta time, not affected by
  // tick system)
  if (m_SceneState == SceneState::Edit) {
    if (m_SceneViewportFocused) {
      // Calculate frame delta for editor camera (not physics)
      float current_time = static_cast<float>(glfwGetTime());
      static float last_editor_time = current_time;
      float editor_dt = current_time - last_editor_time;
      last_editor_time = current_time;

      m_EditorCameraController->OnUpdate(Timestep(editor_dt));
    }

    // Sync Game Camera to Player Entity (Edit Mode)
    for (auto &entity : m_Scene->GetEntities()) {
      if (entity->Name == "Player") {
        m_Camera->SetPosition(entity->Transform.Position);
        m_Camera->SetYaw(entity->Transform.Rotation.y -
                         90.0f); // -90 deg offset usually for default view
        m_Camera->SetPitch(entity->Transform.Rotation.x);

        float aspect = 1.0f;
        if (m_GameViewportSize.x > 0 && m_GameViewportSize.y > 0)
          aspect = m_GameViewportSize.x / m_GameViewportSize.y;
        m_Camera->SetProjection(entity->CameraFOV, aspect, 0.1f, 100.0f);
        break;
      }
    }

    // Safety: If Right Mouse is released, ensure cursor is unlocked
    if (m_CursorLocked && !Input::IsMouseButtonPressed(1)) {
      m_Window->SetCursorLocked(false);
      m_CursorLocked = false;
      m_EditorCameraController->SetRotationEnabled(false);
    }
  } else if (m_SceneState == SceneState::Play ||
             m_SceneState == SceneState::Pause) {
    // Interpolate camera position for smooth rendering
    // Note: Physics ticks run in UpdateGameTick(), here we only interpolate
    // for display
    glm::vec3 interpolated_position = glm::mix(
        m_PreviousState.player_position, m_CurrentState.player_position, alpha);

    float interpolated_yaw =
        glm::mix(m_PreviousState.yaw, m_CurrentState.yaw, alpha);

    float interpolated_pitch =
        glm::mix(m_PreviousState.pitch, m_CurrentState.pitch, alpha);

    // Apply interpolated values to camera for smooth rendering
    m_Camera->SetPosition(interpolated_position + glm::vec3(0.0f, 1.7f, 0.0f));
    m_Camera->SetYaw(interpolated_yaw);
    m_Camera->SetPitch(interpolated_pitch);
  }

  auto &bodyInterface = PhysicsSystem::GetBodyInterface();
  Ref<Entity> selectedEntity = m_SceneHierarchyPanel->GetSelectedEntity();

  // 1. Scene View Pass
  m_SceneFramebuffer->Bind();
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  Renderer::BeginScene(*m_EditorCamera, m_Sun);
  m_Skybox->Draw(*m_EditorCamera);
  for (auto &entity : m_Scene->GetEntities()) {
    // Real-time Player Sync (during Play/Pause)
    if (entity->Name == "Player" && (m_SceneState == SceneState::Play ||
                                     m_SceneState == SceneState::Pause)) {
      if (auto *pc = entity->GetScript<PlayerController>()) {
        pc->SetSettings(entity->Movement);
        entity->Transform.Position =
            m_Camera->GetPosition() - glm::vec3(0.0f, 1.7f, 0.0f);
        entity->Transform.Rotation.x = pc->GetPitch();
        entity->Transform.Rotation.y = pc->GetYaw() + 90.0f;
      }
      // No break here, we need to render it too
    }

    if (entity->Name != "Player" && !entity->PhysicsBody.IsInvalid()) {
      if (m_SceneState == SceneState::Play) {
        JPH::RVec3 position;
        JPH::Quat rotation;
        bodyInterface.GetPositionAndRotation(entity->PhysicsBody, position,
                                             rotation);
        entity->Transform.Position = {position.GetX(), position.GetY(),
                                      position.GetZ()};
        glm::quat q = {rotation.GetW(), rotation.GetX(), rotation.GetY(),
                       rotation.GetZ()};
        entity->Transform.Rotation = glm::degrees(glm::eulerAngles(q));
      } else {
        glm::quat q = glm::quat(glm::radians(entity->Transform.Rotation));
        bodyInterface.SetPositionAndRotation(
            entity->PhysicsBody,
            JPH::RVec3(entity->Transform.Position.x,
                       entity->Transform.Position.y,
                       entity->Transform.Position.z),
            JPH::Quat(q.x, q.y, q.z, q.w), JPH::EActivation::DontActivate);
      }
    }

    if (entity == selectedEntity) {
      glEnable(GL_STENCIL_TEST);
      glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
      glStencilFunc(GL_ALWAYS, 1, 0xFF);
      glStencilMask(0xFF);
    }
    if (entity->Material.AlbedoMap)
      entity->Material.AlbedoMap->Bind();

    if (entity->Mesh && entity->MaterialShader &&
        entity->MaterialShader->IsValid()) {
      Renderer::Submit(entity->MaterialShader, entity->Mesh,
                       entity->Transform.GetTransform(),
                       entity->Material.Tiling);
    }

    if (entity == selectedEntity)
      glStencilMask(0x00);
  }

  if (selectedEntity) {
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glDisable(GL_DEPTH_TEST);
    m_OutlineShader->Bind();
    m_OutlineShader->SetFloat3("u_Color", {1.0f, 0.5f, 0.0f});
    glLineWidth(4.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glm::mat4 transform =
        glm::scale(selectedEntity->Transform.GetTransform(), glm::vec3(1.01f));
    if (selectedEntity->Mesh)
      Renderer::Submit(m_OutlineShader, selectedEntity->Mesh, transform);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glStencilMask(0xFF);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
  }
  Renderer::EndScene();
  m_SceneFramebuffer->Unbind();

  // 2. Game View Pass
  m_GameFramebuffer->Bind();
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  Renderer::BeginScene(*m_Camera, m_Sun);
  m_Skybox->Draw(*m_Camera);
  for (auto &entity : m_Scene->GetEntities()) {
    if (entity->Name == "Player")
      continue; // Hide Player in Game View
    if (entity->Material.AlbedoMap)
      entity->Material.AlbedoMap->Bind();

    if (entity->Mesh && entity->MaterialShader &&
        entity->MaterialShader->IsValid()) {
      Renderer::Submit(entity->MaterialShader, entity->Mesh,
                       entity->Transform.GetTransform(),
                       entity->Material.Tiling);
    }
  }
  Renderer::EndScene();

  // 3. HUD Rendering (only in game viewport)
  HUDRenderer::BeginHUD(m_GameViewportSize.x, m_GameViewportSize.y);
  HUDRenderer::RenderCrosshair();

  if (s_ClShowFPS && s_ClShowFPS->GetBool()) {
      float scale = 4.0f;
      float charHeight = 8.0f * scale;
      float padding = 10.0f;
      glm::vec2 pos = {padding, m_GameViewportSize.y - charHeight - padding};
      HUDRenderer::DrawString("FPS: " + std::to_string((int)m_GameFPS), pos, scale, {0, 1, 0, 1});
  }

  if (m_Scene) {
    if (auto entity = m_Scene->FindEntityByName("Player")) {
      if (auto *pc = entity->GetScript<PlayerController>()) {
        // 1 unit = 0.75 inches
        // 1 meter = 52.4934 Hammer Units
        constexpr float METERS_TO_HU = 52.4934f;
        float speedHU = pc->GetSpeed() * METERS_TO_HU;
        HUDRenderer::RenderSpeed(speedHU);
      }
    }
  }

  HUDRenderer::EndHUD();

  m_GameFramebuffer->Unbind();

  m_ImGuiLayer->Begin();

  if (m_ResetLayoutOnNextFrame) {
    ResetLayout();
    m_ResetLayoutOnNextFrame = false;
  }

  ImGuizmo::BeginFrame();
  {
    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New Project..."))
          OnNewProject();
        if (ImGui::MenuItem("Open Project..."))
          OnOpenProject();

        ImGui::Separator();

        if (ImGui::MenuItem("Open Level...", "Cmd+O"))
          OnOpenScene();

        if (m_LevelLoaded) {
          if (ImGui::MenuItem("Save Level", "Cmd+S"))
            OnSaveScene();
          if (ImGui::MenuItem("Save Level As..."))
            OnSaveSceneAs();
          if (ImGui::MenuItem("Close Level"))
            CloseScene();
        }

        // Project section
        ImGui::Separator();
        
        bool hasProject = !m_ProjectRoot.empty();
        if (ImGui::MenuItem("Build Runtime...", nullptr, false, hasProject))
          OnBuildRuntime();
          
        if (hasProject) {
           if (ImGui::MenuItem("Close Project"))
            CloseProject();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Exit", "Cmd+Q"))
          m_Running = false;

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Settings")) {
        if (ImGui::MenuItem("Settings"))
          m_ShowSettingsWindow = true;
        if (ImGui::MenuItem("Project Settings"))
          m_ShowProjectSettingsWindow = true;
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Window")) {
        if (ImGui::MenuItem("Scene Hierarchy", nullptr, m_ShowHierarchy))
          m_ShowHierarchy = !m_ShowHierarchy;
        if (ImGui::MenuItem("Inspector", nullptr, m_ShowInspector))
          m_ShowInspector = !m_ShowInspector;
        if (ImGui::MenuItem("Content Browser", nullptr, m_ShowContentBrowser))
          m_ShowContentBrowser = !m_ShowContentBrowser;
        if (ImGui::MenuItem("Scene Viewport", nullptr, m_ShowScene))
          m_ShowScene = !m_ShowScene;
        if (ImGui::MenuItem("Game Viewport", nullptr, m_ShowGame))
          m_ShowGame = !m_ShowGame;
        if (ImGui::MenuItem("Toolbar", nullptr, m_ShowToolbar))
          m_ShowToolbar = !m_ShowToolbar;
        if (ImGui::MenuItem("Statistics", nullptr, m_ShowStats))
          m_ShowStats = !m_ShowStats;
        if (ImGui::MenuItem("Developer Console", "`", m_ShowConsole))
          m_ShowConsole = !m_ShowConsole;

        ImGui::Separator();
        if (ImGui::MenuItem("Save Layout"))
          SaveLayout();
        if (ImGui::MenuItem("Load Layout"))
          LoadLayout();
        if (ImGui::MenuItem("Save Layout As...")) {
          std::string filepath = FileDialogs::SaveFile(
              "ImGui Layout (*.ini)\0*.ini\0", "layout.ini", ".ini");
          if (!filepath.empty())
            SaveLayout(filepath);
        }
        if (ImGui::MenuItem("Load Layout From...")) {
          std::string filepath =
              FileDialogs::OpenFile("ImGui Layout (*.ini)\0*.ini\0", ".ini");
          if (!filepath.empty())
            LoadLayout(filepath);
        }
        if (ImGui::MenuItem("Default Layout"))
          ResetLayout();

        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }

    // Hierarchy
    if (m_ShowHierarchy) {
      ImGui::SetNextWindowSizeConstraints(ImVec2(200, 200),
                                          ImVec2(FLT_MAX, FLT_MAX));
      if (!m_ProjectRoot.empty() && m_LevelLoaded) {
        m_SceneHierarchyPanel->OnImGuiRender();

        if (m_SceneHierarchyPanel->GetPendingCreateType() !=
            SceneHierarchyPanel::CreatePrimitiveType::None) {
          auto type = m_SceneHierarchyPanel->GetPendingCreateType();
          m_SceneHierarchyPanel->ClearPendingCreateType();

          Ref<VertexArray> mesh = nullptr;
          std::string name = "Object";
          std::string meshPath = "Cube";

          if (type == SceneHierarchyPanel::CreatePrimitiveType::Cube) {
            mesh = m_CubeMesh;
            name = "Cube";
            meshPath = "Cube";
          } else if (type == SceneHierarchyPanel::CreatePrimitiveType::Sphere) {
            mesh = MeshLoader::LoadOBJ(
                ResolveAssetPath("assets/engine/sphere.obj").string());
            name = "Sphere";
            meshPath = "assets/engine/sphere.obj";
          } else if (type ==
                     SceneHierarchyPanel::CreatePrimitiveType::Cylinder) {
            mesh = MeshLoader::LoadOBJ(
                ResolveAssetPath("assets/engine/cylinder.obj").string());
            name = "Cylinder";
            meshPath = "assets/engine/cylinder.obj";
          }

          if (mesh) {
            auto entity = CreateRef<Entity>(name, mesh, m_DefaultShader,
                                            m_DefaultTexture);
            entity->MeshPath = meshPath;
            glm::vec3 spawnPos = m_EditorCamera->GetPosition() +
                                 m_EditorCamera->GetForward() * 5.0f;
            entity->Transform.Position = spawnPos;

            JPH::BodyCreationSettings settings(
                PhysicsShapes::CreateBox({entity->Transform.Scale.x,
                                          entity->Transform.Scale.y,
                                          entity->Transform.Scale.z}),
                JPH::RVec3(spawnPos.x, spawnPos.y, spawnPos.z),
                JPH::Quat::sIdentity(),
                entity->Anchored ? JPH::EMotionType::Static
                                 : JPH::EMotionType::Dynamic,
                entity->Anchored ? Layers::NON_MOVING : Layers::MOVING);
            settings.mUserData = (uint64_t)entity.get();
            entity->PhysicsBody = bodyInterface.CreateAndAddBody(
                settings, JPH::EActivation::Activate);

            m_Scene->AddEntity(entity);
            m_SceneHierarchyPanel->SetSelectedEntity(entity);
            m_SceneModified = true;
          }
        }
      } else {
        ImGui::Begin("Scene Hierarchy");
        ImGui::End();
      }
    }

    // Content Browser
    if (m_ShowContentBrowser) {
      ImGui::SetNextWindowSizeConstraints(ImVec2(200, 200),
                                          ImVec2(FLT_MAX, FLT_MAX));
      if (!m_ProjectRoot.empty()) {
        m_ContentBrowserPanel->OnImGuiRender();
      } else {
        ImGui::Begin("Content Browser");
        ImGui::End();
      }
    }

    // Inspector
    if (m_ShowInspector) {
      ImGui::SetNextWindowSizeConstraints(ImVec2(200, 200),
                                          ImVec2(FLT_MAX, FLT_MAX));
      ImGui::Begin("Inspector");
      ImGui::End();
    }

    if (m_ShowSettingsWindow)
      UI_SettingsWindow();

    if (m_ShowProjectSettingsWindow)
      UI_ProjectSettingsWindow();

    // Scene Viewport
    if (m_ShowScene) {
      ImGui::SetNextWindowSizeConstraints(ImVec2(300, 200),
                                          ImVec2(FLT_MAX, FLT_MAX));
      ImGui::Begin("Scene");
      m_SceneViewportFocused = ImGui::IsWindowFocused();
      m_SceneViewportHovered = ImGui::IsWindowHovered();

      ImVec2 viewportOffset = ImGui::GetCursorScreenPos();
      m_SceneViewportPos = {viewportOffset.x, viewportOffset.y};
      if (m_SceneState != SceneState::Play)
        m_ImGuiLayer->SetBlockEvents(
            (!m_SceneViewportFocused || !m_SceneViewportHovered) &&
            !ImGuizmo::IsOver());

      ImVec2 sceneSize = ImGui::GetContentRegionAvail();
      m_SceneViewportSize = {sceneSize.x, sceneSize.y};

      if (!m_ProjectRoot.empty() && m_LevelLoaded) {
        ImGui::Image((void *)(uint64_t)
                         m_SceneFramebuffer->GetColorAttachmentRendererID(),
                     sceneSize, {0, 1}, {1, 0});

        // Drag & Drop
        if (ImGui::BeginDragDropTarget()) {
          if (const ImGuiPayload *payload =
                  ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
            const char *path = (const char *)payload->Data;
            std::filesystem::path assetPath = path;

            if (assetPath.extension() == ".obj" ||
                assetPath.extension() == ".stl") {
              Ref<VertexArray> mesh = nullptr;
              if (assetPath.extension() == ".obj")
                mesh = MeshLoader::LoadOBJ(assetPath.string());
              else if (assetPath.extension() == ".stl")
                mesh = MeshLoader::LoadSTL(assetPath.string());

              if (mesh) {
                auto entity =
                    CreateRef<Entity>(assetPath.stem().string(), mesh,
                                      m_DefaultShader, m_DefaultTexture);
                entity->MeshPath = assetPath.string();
                glm::vec3 dropPos = m_EditorCamera->GetPosition() +
                                    m_EditorCamera->GetForward() * 5.0f;
                entity->Transform.Position = dropPos;

                JPH::BodyCreationSettings settings(
                    PhysicsShapes::CreateBox({1.0f, 1.0f, 1.0f}),
                    JPH::RVec3(dropPos.x, dropPos.y, dropPos.z),
                    JPH::Quat::sIdentity(),
                    entity->Anchored ? JPH::EMotionType::Static
                                     : JPH::EMotionType::Dynamic,
                    entity->Anchored ? Layers::NON_MOVING : Layers::MOVING);
                settings.mUserData = (uint64_t)entity.get();
                entity->PhysicsBody = bodyInterface.CreateAndAddBody(
                    settings, JPH::EActivation::Activate);

                m_Scene->AddEntity(entity);
                m_SceneHierarchyPanel->SetSelectedEntity(entity);
                m_SceneModified = true;
              }
            }
          }
          ImGui::EndDragDropTarget();
        }

        // Gizmos
        if (selectedEntity && m_GizmoType != -1) {
          ImGuizmo::SetOrthographic(false);
          ImGuizmo::SetDrawlist();

          ImGuizmo::SetRect(m_SceneViewportPos.x, m_SceneViewportPos.y,
                            m_SceneViewportSize.x, m_SceneViewportSize.y);

          // Editor camera
          const glm::mat4 &cameraProjection =
              m_EditorCamera->GetProjectionMatrix();
          glm::mat4 cameraView = m_EditorCamera->GetViewMatrix();

          // Entity transform
          glm::mat4 transform = selectedEntity->Transform.GetTransform();

          // Snapping
          bool snap = Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL) ||
                      Input::IsKeyPressed(GLFW_KEY_LEFT_SUPER);
          float snapValue = 0.5f; // Snap to 0.5m for translation/scale
          if (m_GizmoType == (int)ImGuizmo::OPERATION::ROTATE)
            snapValue = 45.0f;

          float snapValues[3] = {snapValue, snapValue, snapValue};

          ImGuizmo::Manipulate(
              glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
              (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL,
              glm::value_ptr(transform), nullptr, snap ? snapValues : nullptr);

          if (ImGuizmo::IsUsing()) {
            if (!m_IsDraggingGizmo) {
              m_IsDraggingGizmo = true;
              m_InitialGizmoTransform = selectedEntity->Transform;
            }

            float translation[3], rotation[3], scale[3];
            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform),
                                                  translation, rotation, scale);

            selectedEntity->Transform.Position = {
                translation[0], translation[1], translation[2]};
            selectedEntity->Transform.Rotation = {rotation[0], rotation[1],
                                                  rotation[2]};
            selectedEntity->Transform.Scale = {scale[0], scale[1], scale[2]};
          } else {
            if (m_IsDraggingGizmo) {
              m_IsDraggingGizmo = false;
              m_UndoSystem.AddCommand(CreateScope<TransformCommand>(
                  selectedEntity, m_InitialGizmoTransform,
                  selectedEntity->Transform));
              m_SceneModified = true;
            }
          }
        }
      } else {
        std::string text =
            m_ProjectRoot.empty() ? "No project open" : "No level open";
        ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
        ImGui::SetCursorPos({(sceneSize.x - textSize.x) * 0.5f,
                             (sceneSize.y - textSize.y) * 0.5f});
        ImGui::Text("%s", text.c_str());
      }

      // Save notification popup (bottom-left corner)
      if (m_ShowSaveNotification) {
        float currentTime = (float)glfwGetTime();
        float elapsed = currentTime - m_SaveNotificationTime;

        if (elapsed < 3.0f) {
          // Position in bottom-left corner
          ImVec2 windowPos = ImGui::GetWindowPos();
          ImVec2 windowSize = ImGui::GetWindowSize();
          ImVec2 notificationSize = {200.0f, 50.0f};
          ImVec2 padding = {10.0f, 10.0f};

          ImGui::SetNextWindowPos(
              {windowPos.x + padding.x,
               windowPos.y + windowSize.y - notificationSize.y - padding.y});
          ImGui::SetNextWindowSize(notificationSize);

          // Fade out in the last 0.5 seconds
          float fadeAlpha = (elapsed > 2.5f) ? (3.0f - elapsed) / 0.5f : 1.0f;
          ImGui::PushStyleVar(ImGuiStyleVar_Alpha, fadeAlpha);

          // Use standard theme background color (no PushStyleColor)

          ImGui::Begin("##SaveNotification", nullptr,
                       ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                           ImGuiWindowFlags_NoMove |
                           ImGuiWindowFlags_NoScrollbar |
                           ImGuiWindowFlags_NoInputs);

          // Center text
          const char *notifText = "Scene Saved!";
          ImVec2 notifTextSize = ImGui::CalcTextSize(notifText);
          ImGui::SetCursorPos({(notificationSize.x - notifTextSize.x) * 0.5f,
                               (notificationSize.y - notifTextSize.y) * 0.5f});
          ImGui::Text("%s", notifText);

          ImGui::End();
          ImGui::PopStyleVar();
        } else {
          m_ShowSaveNotification = false;
        }
      }

      ImGui::End();
    }

    // Game Viewport
    if (m_ShowGame) {
      ImGui::SetNextWindowSizeConstraints(ImVec2(300, 200),
                                          ImVec2(FLT_MAX, FLT_MAX));
      ImGui::Begin("Game");
      m_GameViewportFocused = ImGui::IsWindowFocused();
      m_GameViewportHovered = ImGui::IsWindowHovered();
      if (m_SceneState == SceneState::Play)
        m_ImGuiLayer->SetBlockEvents(!m_GameViewportFocused ||
                                     !m_GameViewportHovered);

      ImVec2 gameSize = ImGui::GetContentRegionAvail();
      m_GameViewportSize = {gameSize.x, gameSize.y};

      if (!m_ProjectRoot.empty() && m_LevelLoaded) {
        ImGui::Image(
            (void *)(uint64_t)m_GameFramebuffer->GetColorAttachmentRendererID(),
            gameSize, {0, 1}, {1, 0});
      } else {
        std::string text =
            m_ProjectRoot.empty() ? "No project open" : "No level open";
        ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
        ImGui::SetCursorPos({(gameSize.x - textSize.x) * 0.5f,
                             (gameSize.y - textSize.y) * 0.5f});
        ImGui::Text("%s", text.c_str());
      }
      ImGui::End();
    }

    if (m_ShowToolbar) {
      ImGui::SetNextWindowSizeConstraints(ImVec2(200, 50),
                                          ImVec2(FLT_MAX, FLT_MAX));
      ImGui::Begin("Toolbar");

      if (m_SceneState == SceneState::Edit) {
        if (ImGui::Button("Play")) {
          OnScenePlay();
          ImGui::SetWindowFocus("Game");
        }
      } else if (m_SceneState == SceneState::Pause) {
        if (ImGui::Button("Resume")) {
          OnScenePlay();
          ImGui::SetWindowFocus("Game");
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop"))
          OnSceneStop();
      } else {
        if (ImGui::Button("Pause"))
          OnScenePause();
        ImGui::SameLine();
        if (ImGui::Button("Stop"))
          OnSceneStop();
      }
      ImGui::End();
    }
  }

  if (m_ShowStats) {
    if (!m_ProjectRoot.empty()) {
      ImGui::Begin("Engine Statistics");
      float speed = 0.0f;
      glm::vec3 vel(0.0f);
      if (m_Scene) {
        if (auto entity = m_Scene->FindEntityByName("Player")) {
          if (auto *pc = entity->GetScript<PlayerController>()) {
            speed = pc->GetSpeed();
            vel = pc->GetVelocity();
          }
        }
      }

      ImGui::Text("%.3f ms/frame (%.1f Game FPS | %.1f Engine FPS)",
                  1000.0f / m_GameFPS, m_GameFPS, ImGui::GetIO().Framerate);
      ImGui::Separator();
      // Convert meters to Hammer Units (1 unit = 0.75 inches -> 1 meter
      // = 52.4934 HU)
      constexpr float METERS_TO_HU = 52.4934f;
      ImGui::Text("Velocity:  X: %.2f  Y: %.2f  Z: %.2f", vel.x * METERS_TO_HU,
                  vel.y * METERS_TO_HU, vel.z * METERS_TO_HU);
      ImGui::Text("Speed (H): %.2f units/s", speed * METERS_TO_HU);
      ImGui::End();
    } else {
      ImGui::SetNextWindowSizeConstraints(ImVec2(200, 100),
                                          ImVec2(FLT_MAX, FLT_MAX));
      ImGui::Begin("Engine Statistics");
      ImGui::End();
    }
  }

  if (m_ConsolePanel)
    m_ConsolePanel->OnImGuiRender(&m_ShowConsole);

  if (m_ProjectRoot.empty()) {
    UI_LauncherScreen();
  }
  // Unsaved Changes Modal Dialogs
  if (ImGui::BeginPopupModal("Unsaved Changes##NewScene", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("You have unsaved changes. What would you like to do?");
    ImGui::Separator();

    if (ImGui::Button("Save and Continue", ImVec2(150, 0))) {
      OnSaveScene();
      m_SceneModified = false;
      ImGui::CloseCurrentPopup();
      OnNewScene();
    }
    ImGui::SameLine();
    if (ImGui::Button("Discard Changes", ImVec2(150, 0))) {
      m_SceneModified = false;
      ImGui::CloseCurrentPopup();
      OnNewScene();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(150, 0))) {
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }

  if (ImGui::BeginPopupModal("Unsaved Changes##OpenScene", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("You have unsaved changes. What would you like to do?");
    ImGui::Separator();

    if (ImGui::Button("Save and Continue", ImVec2(150, 0))) {
      OnSaveScene();
      m_SceneModified = false;
      ImGui::CloseCurrentPopup();
      if (!m_PendingScenePath.empty()) {
        std::string path = m_PendingScenePath;
        m_PendingScenePath.clear();
        OpenScene(path);
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Discard Changes", ImVec2(150, 0))) {
      m_SceneModified = false;
      ImGui::CloseCurrentPopup();
      if (!m_PendingScenePath.empty()) {
        std::string path = m_PendingScenePath;
        m_PendingScenePath.clear();
        OpenScene(path);
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(150, 0))) {
      m_PendingScenePath.clear();
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }

  if (ImGui::BeginPopupModal("Unsaved Changes##OpenSceneDirect", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("You have unsaved changes. What would you like to do?");
    ImGui::Separator();

    if (ImGui::Button("Save and Continue", ImVec2(150, 0))) {
      OnSaveScene();
      m_SceneModified = false;
      ImGui::CloseCurrentPopup();
      if (!m_PendingScenePath.empty()) {
        std::string path = m_PendingScenePath;
        m_PendingScenePath.clear();
        OpenScene(path);
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Discard Changes", ImVec2(150, 0))) {
      m_SceneModified = false;
      ImGui::CloseCurrentPopup();
      if (!m_PendingScenePath.empty()) {
        std::string path = m_PendingScenePath;
        m_PendingScenePath.clear();
        OpenScene(path);
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(150, 0))) {
      m_PendingScenePath.clear();
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }

  // Auto-save system (every 60 seconds)
  if (m_SceneState == SceneState::Edit && m_LevelLoaded &&
      !m_LevelFilePath.empty() && m_LevelFilePath != "Untitled.s67") {
    float current_time = static_cast<float>(glfwGetTime());
    if (current_time - m_LastAutoSaveTime >= 60.0f) {
      SceneSerializer serializer(m_Scene.get(), m_ProjectRoot.string());
      serializer.Serialize(m_LevelFilePath);
      m_LastAutoSaveTime = current_time;
      S67_CORE_INFO("Auto-saved level: {0}", m_LevelFilePath);
    }
  }

  m_ImGuiLayer->End();
#endif
}

} // namespace S67
