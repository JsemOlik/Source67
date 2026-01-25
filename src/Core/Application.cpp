#include "Application.h"
#include "Logger.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Renderer/Renderer.h"
#include "Renderer/Shader.h"
#include "Renderer/Buffer.h"
#include "Renderer/VertexArray.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <imgui.h>
#include "Renderer/Framebuffer.h"
#include "Renderer/SceneSerializer.h"
#include "Physics/PhysicsShapes.h"
#include "Core/PlatformUtils.h"
#include "Core/Input.h"
#include "ImGui/Panels/ContentBrowserPanel.h"
#include "ImGuizmo/ImGuizmo.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/transform.hpp>
#include "Renderer/Mesh.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <iomanip>

namespace S67 {

    struct SceneBackup {
        struct TransformData {
            glm::vec3 Position, Rotation, Scale;
        };
        std::unordered_map<void*, TransformData> Data;
    };
    static SceneBackup s_SceneBackup;

    Application* Application::s_Instance = nullptr;

    Application::Application(const std::string& executablePath) {
        S67_CORE_ASSERT(!s_Instance, "Application already exists!");
        s_Instance = this;

        // Find assets root
        std::filesystem::path currentPath = std::filesystem::absolute(executablePath).parent_path();
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
            S67_CORE_ERROR("Could not find 'assets' directory relative to executable path: {0}!", executablePath);
        } else {
            S67_CORE_INFO("Set working directory to project root: {0}", currentPath.string());
        }

        S67_CORE_INFO("Initializing Window...");
        m_Window = std::unique_ptr<Window>(Window::Create());
        m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

        S67_CORE_INFO("Initializing Renderer...");
        Renderer::Init();

        S67_CORE_INFO("Initializing Physics...");
        PhysicsSystem::Init();

        m_Camera = CreateRef<PerspectiveCamera>(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
        m_Camera->SetPosition({ 0.0f, 2.0f, 8.0f });
        m_PlayerController = CreateScope<PlayerController>(m_Camera);

        m_EditorCamera = CreateRef<PerspectiveCamera>(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
        m_EditorCamera->SetPosition({ 5.0f, 5.0f, 15.0f });

        m_Scene = CreateScope<Scene>();
        m_Sun.Direction = { -0.5f, -1.0f, -0.2f };
        m_Sun.Color = { 1.0f, 0.95f, 0.8f }; // Warm sun color
        m_Sun.Intensity = 1.0f;

        CreateTestScene();

        m_CameraController = CreateRef<CameraController>(m_Camera);
        m_EditorCameraController = CreateRef<CameraController>(m_EditorCamera);
        m_EditorCameraController->SetRotationEnabled(false); // Only via Right-Click
        m_Window->SetCursorLocked(false);

        m_ImGuiLayer = CreateScope<ImGuiLayer>();
        m_ImGuiLayer->OnAttach();

        m_SceneHierarchyPanel = CreateScope<SceneHierarchyPanel>(m_Scene);
        m_ContentBrowserPanel = CreateScope<ContentBrowserPanel>();
        m_Skybox = CreateScope<Skybox>("assets/textures/skybox.png");
        LoadSettings();

        FramebufferSpecification fbSpec;
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_SceneFramebuffer = Framebuffer::Create(fbSpec);
        m_GameFramebuffer = Framebuffer::Create(fbSpec);
        m_OutlineShader = Shader::Create("assets/shaders/FlatColor.glsl");

        S67_CORE_INFO("Application initialized successfully");
    }

    void Application::CreateTestScene() {
        S67_CORE_INFO("Setting up test scene...");
        auto vertexArray = VertexArray::Create();

        float vertices[] = {
            // Position           // Normals           // TexCoords
            // Front
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
            // Back
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
            // Top
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
            // Bottom
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
            // Left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            // Right
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f
        };

        Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
        vertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float3, "a_Normal" },
            { ShaderDataType::Float2, "a_TexCoord" }
        });
        vertexArray->AddVertexBuffer(vertexBuffer);

        uint32_t indices[] = {
            0,  1,  2,  2,  3,  0,
            4,  5,  6,  6,  7,  4,
            8,  9,  10, 10, 11, 8,
            12, 13, 14, 14, 15, 12,
            16, 17, 18, 18, 19, 16,
            20, 21, 22, 22, 23, 20
        };
        vertexArray->SetIndexBuffer(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));

        auto shader = Shader::Create("assets/shaders/Lighting.glsl");
        auto texture = Texture2D::Create("assets/textures/Checkerboard.png");
        m_DefaultShader = shader;
        m_DefaultTexture = texture;

        auto& bodyInterface = PhysicsSystem::GetBodyInterface();

        // 1. Static Floor
        auto floorVA = vertexArray; // Reusing cube for floor
        auto floor = CreateRef<Entity>("Static Floor", floorVA, shader, texture);
        floor->Transform.Position = { 0.0f, -2.0f, 0.0f };
        floor->Transform.Scale = { 20.0f, 1.0f, 20.0f };
        
        JPH::BodyCreationSettings floorSettings(PhysicsShapes::CreateBox({ 20.0f, 1.0f, 20.0f }), JPH::RVec3(0, -2, 0), JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::NON_MOVING);
        floorSettings.mUserData = (uint64_t)floor.get();
        floor->PhysicsBody = bodyInterface.CreateAndAddBody(floorSettings, JPH::EActivation::DontActivate);
        m_Scene->AddEntity(floor);

        // 2. Dynamic Cubes
        for (int i = 0; i < 5; i++) {
            std::string name = "Cube " + std::to_string(i);
            auto cube = CreateRef<Entity>(name, vertexArray, shader, texture);
            cube->Transform.Position = { (float)i * 2.0f - 4.0f, 10.0f + (float)i * 2.0f, 0.0f };
            
            JPH::BodyCreationSettings cubeSettings(PhysicsShapes::CreateBox({ 1.0f, 1.0f, 1.0f }), JPH::RVec3(cube->Transform.Position.x, cube->Transform.Position.y, cube->Transform.Position.z), JPH::Quat::sIdentity(), JPH::EMotionType::Dynamic, Layers::MOVING);
            cubeSettings.mUserData = (uint64_t)cube.get();
            cube->PhysicsBody = bodyInterface.CreateAndAddBody(cubeSettings, JPH::EActivation::Activate);
            m_Scene->AddEntity(cube);
        }
    }

    void Application::ResetScene() {
        S67_CORE_INFO("Resetting scene...");
        OnSceneStop();

        // Clear Physics
        auto& bodyInterface = PhysicsSystem::GetBodyInterface();
        for (auto& entity : m_Scene->GetEntities()) {
            if (!entity->PhysicsBody.IsInvalid()) {
                bodyInterface.RemoveBody(entity->PhysicsBody);
                bodyInterface.DestroyBody(entity->PhysicsBody);
            }
        }

        // Re-create
        m_Scene = CreateScope<Scene>();
        m_UndoSystem.Clear();
        m_IsDraggingGizmo = false;
        CreateTestScene();

        if (m_SceneHierarchyPanel) {
            m_SceneHierarchyPanel->SetContext(m_Scene);
        }
    }

    Application::~Application() {
        m_ImGuiLayer->OnDetach();
        PhysicsSystem::Shutdown();
    }

    void Application::OnScenePlay() {
        if (m_SceneState == SceneState::Edit) {
            // Backup before first play
            s_SceneBackup.Data.clear();
            for (auto& entity : m_Scene->GetEntities()) {
                s_SceneBackup.Data[entity.get()] = { entity->Transform.Position, entity->Transform.Rotation, entity->Transform.Scale };
            }
        }

        m_SceneState = SceneState::Play;
        m_Window->SetCursorLocked(true);

        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
        m_ImGuiLayer->SetBlockEvents(false);
        m_PlayerController->Reset({ 0.0f, 2.0f, 8.0f });
    }

    void Application::OnScenePause() {
        if (m_SceneState != SceneState::Play) return;

        m_SceneState = SceneState::Pause;
        m_Window->SetCursorLocked(false);

        ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        m_ImGuiLayer->SetBlockEvents(true);
    }

    void Application::OnSceneStop() {
        m_SceneState = SceneState::Edit;
        m_Window->SetCursorLocked(false);

        ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        m_ImGuiLayer->SetBlockEvents(true);

        // Restore
        auto& bodyInterface = PhysicsSystem::GetBodyInterface();
        for (auto& entity : m_Scene->GetEntities()) {
            if (s_SceneBackup.Data.count(entity.get())) {
                auto& data = s_SceneBackup.Data[entity.get()];
                entity->Transform.Position = data.Position;
                entity->Transform.Rotation = data.Rotation;
                entity->Transform.Scale = data.Scale;

                if (!entity->PhysicsBody.IsInvalid()) {
                    glm::quat q = glm::quat(glm::radians(entity->Transform.Rotation));
                    JPH::Quat jRotate(q.x, q.y, q.z, q.w);
                    bodyInterface.SetPositionAndRotation(entity->PhysicsBody, JPH::RVec3(data.Position.x, data.Position.y, data.Position.z), jRotate, JPH::EActivation::DontActivate);
                    bodyInterface.SetLinearAndAngularVelocity(entity->PhysicsBody, JPH::Vec3::sZero(), JPH::Vec3::sZero());
                }
            }
        }
    }

    void Application::SetProjectRoot(const std::filesystem::path& root) {
        m_ProjectRoot = root;
        if (m_ContentBrowserPanel)
            m_ContentBrowserPanel->SetRoot(root);
    }

    void Application::OnNewProject() {
        std::string path = FileDialogs::SaveFile("Source67 Manifest (manifest.json)\0manifest.json\0", "manifest", "json");
        if (!path.empty()) {
            std::filesystem::path manifestPath(path);
            std::filesystem::create_directories(manifestPath.parent_path());
            
            m_ProjectName = manifestPath.parent_path().stem().string();
            m_ProjectVersion = "1.0.0";

            // Save Manifest File
            std::ofstream fout(path);
            fout << "ProjectName: " << m_ProjectName << "\n";
            fout << "Version: " << m_ProjectVersion << "\n";
            fout.close();

            SetProjectRoot(manifestPath.parent_path());
            m_ProjectFilePath = manifestPath;
            S67_CORE_INFO("Created new project manifest: {0}", path);
        }
    }

    void Application::OnOpenProject() {
        std::string path = FileDialogs::OpenFolder();
        if (!path.empty()) {
            std::filesystem::path folderPath(path);
            SetProjectRoot(folderPath);
            
            // Try to find a manifest in this specific folder
            std::filesystem::path manifestPath = folderPath / "manifest.json";
            if (std::filesystem::exists(manifestPath)) {
                // If opening a level later, it will discover properly, but let's load it now too
                DiscoverProject(manifestPath); // Discovery takes a "level" path essentially
            } else {
                m_ProjectName = folderPath.stem().string();
                m_ProjectVersion = "Developer Root";
                m_ProjectFilePath = "";
            }
            S67_CORE_INFO("Opened project folder: {0}", path);
        }
    }

    void Application::DiscoverProject(const std::filesystem::path& levelPath) {
        std::filesystem::path currentDir = levelPath.parent_path();
        bool found = false;

        // Search upward for manifest.json
        while (!currentDir.empty() && currentDir != currentDir.root_path()) {
            std::filesystem::path manifestPath = currentDir / "manifest.json";
            if (std::filesystem::exists(manifestPath)) {
                m_ProjectFilePath = manifestPath;
                SetProjectRoot(currentDir);
                
                // Parse Manifest
                std::ifstream fin(manifestPath);
                std::string line;
                while (std::getline(fin, line)) {
                    if (!line.empty() && line.back() == '\r') line.pop_back();
                    if (line.find("ProjectName:") != std::string::npos) {
                        m_ProjectName = line.substr(line.find(":") + 2);
                    } else if (line.find("Version:") != std::string::npos) {
                        m_ProjectVersion = line.substr(line.find(":") + 2);
                    }
                }
                S67_CORE_INFO("Discovered project: {0} (v{1}) at {2}", m_ProjectName, m_ProjectVersion, currentDir.string());
                found = true;
                break;
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
        ResetScene();
        m_LevelLoaded = true;
        m_LevelFilePath = "Untitled.s67";
        S67_CORE_INFO("Created new level");
    }

    void Application::CloseScene() {
        ResetScene();
        m_LevelLoaded = false;
        m_LevelFilePath = "";
        m_ProjectName = "Standalone";
        m_ProjectVersion = "N/A";
        S67_CORE_INFO("Closed level");
    }

    void Application::OnSaveScene() {
        if (m_SceneState != SceneState::Edit) {
            S67_CORE_WARN("Cannot save while playing!");
            return;
        }

        if (m_LevelLoaded && !m_LevelFilePath.empty() && m_LevelFilePath != "Untitled.s67") {
            SceneSerializer serializer(m_Scene.get());
            serializer.Serialize(m_LevelFilePath);
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

        std::string defaultName = m_LevelLoaded ? std::filesystem::path(m_LevelFilePath).stem().string() : "level";
        std::string filepath = FileDialogs::SaveFile("Source67 Level (*.s67)\0*.s67\0", defaultName.c_str(), "s67");
        if (!filepath.empty()) {
            SceneSerializer serializer(m_Scene.get());
            serializer.Serialize(filepath);
            m_LevelLoaded = true;
            m_LevelFilePath = filepath;
            DiscoverProject(std::filesystem::path(filepath));
        }
    }

    void Application::OnOpenScene() {
        if (m_SceneState != SceneState::Edit) {
            S67_CORE_WARN("Cannot load while playing!");
            return;
        }

        std::string filepath = FileDialogs::OpenFile("Source67 Level (*.s67)\0*.s67\0", "s67");
        if (!filepath.empty()) {
            OpenScene(filepath);
        }
    }

    void Application::OpenScene(const std::string& filepath) {
        PhysicsSystem::Shutdown(); // Reset physics system to clear all bodies
        PhysicsSystem::Init();
        m_PlayerController = CreateScope<PlayerController>(m_Camera);

        SceneSerializer serializer(m_Scene.get());
        if (serializer.Deserialize(filepath)) {
            m_LevelLoaded = true;
            m_LevelFilePath = filepath;
            DiscoverProject(std::filesystem::path(filepath));
            auto& bodyInterface = PhysicsSystem::GetBodyInterface();
            
            // Shared Mesh for now
            auto vertexArray = VertexArray::Create();
            float vertices[] = {
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
                 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
                 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
                -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
                 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
                 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
                -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
                -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
                 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
                 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
                -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
                -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
                 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
                 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
                -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
                -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
                 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
                 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
                 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
                 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f
            };
            auto vbo = VertexBuffer::Create(vertices, sizeof(vertices));
            vbo->SetLayout({ { ShaderDataType::Float3, "a_Position" }, { ShaderDataType::Float3, "a_Normal" }, { ShaderDataType::Float2, "a_TexCoord" } });
            vertexArray->AddVertexBuffer(vbo);
            uint32_t indices[] = { 0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4, 8, 9, 10, 10, 11, 8, 12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20 };
            auto ibo = IndexBuffer::Create(indices, 36);
            vertexArray->SetIndexBuffer(ibo);

            for (auto& entity : m_Scene->GetEntities()) {
                entity->Mesh = vertexArray;
                
                // Recreate Physics Body
                glm::quat q = glm::quat(glm::radians(entity->Transform.Rotation));
                bool isStat = (entity->Name == "Static Floor");
                JPH::BodyCreationSettings settings(PhysicsShapes::CreateBox({ entity->Transform.Scale.x, entity->Transform.Scale.y, entity->Transform.Scale.z }), 
                    JPH::RVec3(entity->Transform.Position.x, entity->Transform.Position.y, entity->Transform.Position.z), 
                    JPH::Quat(q.x, q.y, q.z, q.w), 
                    isStat ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic, 
                    isStat ? Layers::NON_MOVING : Layers::MOVING);
                
                entity->PhysicsBody = bodyInterface.CreateAndAddBody(settings, JPH::EActivation::Activate);
            }
        }
    }

    void Application::OnEntityCollidableChanged(Ref<Entity> entity) {
        if (!entity) return;

        auto& bodyInterface = PhysicsSystem::GetBodyInterface();
        if (!entity->PhysicsBody.IsInvalid()) {
            bodyInterface.RemoveBody(entity->PhysicsBody);
            bodyInterface.DestroyBody(entity->PhysicsBody);
        }

        glm::quat q = glm::quat(glm::radians(entity->Transform.Rotation));
        bool isStatic = (entity->Name == "Static Floor");
        
        JPH::BodyCreationSettings settings(PhysicsShapes::CreateBox({ entity->Transform.Scale.x, entity->Transform.Scale.y, entity->Transform.Scale.z }), 
            JPH::RVec3(entity->Transform.Position.x, entity->Transform.Position.y, entity->Transform.Position.z), 
            JPH::Quat(q.x, q.y, q.z, q.w), 
            isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic, 
            isStatic ? Layers::NON_MOVING : Layers::MOVING);
        
        settings.mUserData = (uint64_t)entity.get();

        entity->PhysicsBody = bodyInterface.CreateAndAddBody(settings, JPH::EActivation::Activate);
    }

    void Application::OnEvent(Event& e) {
        m_ImGuiLayer->OnEvent(e);
        
        if (m_SceneState == SceneState::Play) {
            // m_CameraController->OnEvent(e);
            m_PlayerController->OnEvent(e);
        } else {
            // Editor Navigation logic
            if (e.GetEventType() == EventType::MouseButtonPressed) {
                auto& mb = (MouseButtonPressedEvent&)e;
                if (mb.GetMouseButton() == 1) { // Right Click
                    if (m_SceneViewportHovered) {
                        m_Window->SetCursorLocked(true);
                        m_EditorCameraController->SetRotationEnabled(true);
                        m_EditorCameraController->SetFirstMouse(true);
                    }
                } else if (mb.GetMouseButton() == 0) { // Left Click
                    if (m_SceneViewportHovered && m_SceneState != SceneState::Play) {
                        if (ImGuizmo::IsOver()) return;
                        
                        // Mouse Picking
                        ImVec2 mousePos = ImGui::GetMousePos();
                        float x = mousePos.x - m_SceneViewportPos.x;
                        float y = mousePos.y - m_SceneViewportPos.y;

                        // NDC (-1 to 1)
                        float ndcX = (2.0f * x) / m_SceneViewportSize.x - 1.0f;
                        float ndcY = 1.0f - (2.0f * y) / m_SceneViewportSize.y;

                        glm::mat4 invVP = glm::inverse(m_EditorCamera->GetViewProjectionMatrix());
                        glm::vec4 ndcRayNear = { ndcX, ndcY, -1.0f, 1.0f };
                        glm::vec4 ndcRayFar = { ndcX, ndcY, 1.0f, 1.0f };

                        glm::vec4 worldRayNear = invVP * ndcRayNear;
                        glm::vec4 worldRayFar = invVP * ndcRayFar;

                        worldRayNear /= worldRayNear.w;
                        worldRayFar /= worldRayFar.w;

                        glm::vec3 rayOrigin = glm::vec3(worldRayNear);
                        glm::vec3 rayDirection = glm::normalize(glm::vec3(worldRayFar - worldRayNear));

                        JPH::BodyID hitID = PhysicsSystem::Raycast(rayOrigin, rayDirection, 1000.0f);
                        if (!hitID.IsInvalid()) {
                            for (auto& entity : m_Scene->GetEntities()) {
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
            } else if (e.GetEventType() == EventType::MouseButtonReleased) {
                auto& mb = (MouseButtonReleasedEvent&)e;
                if (mb.GetMouseButton() == 1) { // Right Click
                    m_Window->SetCursorLocked(false);
                    m_EditorCameraController->SetRotationEnabled(false);
                }
            }
            
            m_EditorCameraController->OnEvent(e);
        }

        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));
        dispatcher.Dispatch<WindowDropEvent>(BIND_EVENT_FN(Application::OnWindowDrop));

        if (e.GetEventType() == EventType::KeyPressed) {
            auto& ek = (KeyPressedEvent&)e;
            bool control = Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL) || Input::IsKeyPressed(GLFW_KEY_RIGHT_CONTROL);
            bool super = Input::IsKeyPressed(GLFW_KEY_LEFT_SUPER) || Input::IsKeyPressed(GLFW_KEY_RIGHT_SUPER);
            
            if (m_SceneState == SceneState::Edit) {
                if (ek.GetKeyCode() == GLFW_KEY_S && (control || super)) {
                    OnSaveScene();
                }

                // Undo/Redo
                if (control || super) {
                    if (ek.GetKeyCode() == GLFW_KEY_Z) {
                        if (Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT) || Input::IsKeyPressed(GLFW_KEY_RIGHT_SHIFT))
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
                        float maxScale = std::max({ scale.x, scale.y, scale.z });
                        
                        // Move camera to look at object
                        glm::vec3 offset = { 0.0f, maxScale * 2.0f, maxScale * 5.0f };
                        m_EditorCamera->SetPosition(pos + offset);
                        
                        // We could also set rotation here, but let's keep it simple first
                    }
                }
            }

            if (ek.GetKeyCode() == GLFW_KEY_ESCAPE) {
                if (m_SceneState == SceneState::Play)
                    OnScenePause();
            }
        }
    }

    void Application::Run() {
        while (m_Running) {
            float time = (float)glfwGetTime();
            Timestep timestep = time - m_LastFrameTime;
            m_LastFrameTime = time;

            // Viewport Resize
            if (FramebufferSpecification spec = m_SceneFramebuffer->GetSpecification();
                m_SceneViewportSize.x > 0.0f && m_SceneViewportSize.y > 0.0f &&
                (spec.Width != (uint32_t)m_SceneViewportSize.x || spec.Height != (uint32_t)m_SceneViewportSize.y)) {
                m_SceneFramebuffer->Resize((uint32_t)m_SceneViewportSize.x, (uint32_t)m_SceneViewportSize.y);
                m_EditorCamera->SetProjection(45.0f, m_SceneViewportSize.x / m_SceneViewportSize.y, 0.1f, 100.0f);
            }
            if (FramebufferSpecification spec = m_GameFramebuffer->GetSpecification();
                m_GameViewportSize.x > 0.0f && m_GameViewportSize.y > 0.0f &&
                (spec.Width != (uint32_t)m_GameViewportSize.x || spec.Height != (uint32_t)m_GameViewportSize.y)) {
                m_GameFramebuffer->Resize((uint32_t)m_GameViewportSize.x, (uint32_t)m_GameViewportSize.y);
                m_Camera->SetProjection(45.0f, m_GameViewportSize.x / m_GameViewportSize.y, 0.1f, 100.0f);
            }

            if (m_SceneState == SceneState::Play) {
                // m_CameraController->OnUpdate(timestep); // Disable default fly cam movement
                m_PlayerController->OnUpdate(timestep);
                PhysicsSystem::OnUpdate(timestep);
            } else if (m_SceneViewportFocused) {
                m_EditorCameraController->OnUpdate(timestep);
            }

            auto& bodyInterface = PhysicsSystem::GetBodyInterface();
            Ref<Entity> selectedEntity = m_SceneHierarchyPanel->GetSelectedEntity();

            // 1. Scene View Pass
            m_SceneFramebuffer->Bind();
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            Renderer::BeginScene(*m_EditorCamera, m_Sun);
            m_Skybox->Draw(*m_EditorCamera);
            for (auto& entity : m_Scene->GetEntities()) {
                if (!entity->PhysicsBody.IsInvalid()) {
                    if (m_SceneState == SceneState::Play) {
                        JPH::RVec3 position; JPH::Quat rotation;
                        bodyInterface.GetPositionAndRotation(entity->PhysicsBody, position, rotation);
                        entity->Transform.Position = { position.GetX(), position.GetY(), position.GetZ() };
                        glm::quat q = { rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ() };
                        entity->Transform.Rotation = glm::degrees(glm::eulerAngles(q));
                    } else {
                        glm::quat q = glm::quat(glm::radians(entity->Transform.Rotation));
                        bodyInterface.SetPositionAndRotation(entity->PhysicsBody, JPH::RVec3(entity->Transform.Position.x, entity->Transform.Position.y, entity->Transform.Position.z), JPH::Quat(q.x, q.y, q.z, q.w), JPH::EActivation::DontActivate);
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
                Renderer::Submit(entity->MaterialShader, entity->Mesh, entity->Transform.GetTransform(), entity->Material.Tiling);
                if (entity == selectedEntity) glStencilMask(0x00);
            }

            if (selectedEntity) {
                glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
                glDisable(GL_DEPTH_TEST);
                m_OutlineShader->Bind();
                m_OutlineShader->SetFloat3("u_Color", { 1.0f, 0.5f, 0.0f });
                glLineWidth(4.0f);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glm::mat4 transform = glm::scale(selectedEntity->Transform.GetTransform(), glm::vec3(1.01f));
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
            for (auto& entity : m_Scene->GetEntities()) {
                if (entity->Material.AlbedoMap)
                    entity->Material.AlbedoMap->Bind();
                Renderer::Submit(entity->MaterialShader, entity->Mesh, entity->Transform.GetTransform(), entity->Material.Tiling);
            }
            Renderer::EndScene();
            m_GameFramebuffer->Unbind();

            m_ImGuiLayer->Begin();
            ImGuizmo::BeginFrame();
            {
                if (ImGui::BeginMainMenuBar()) {
                    if (ImGui::BeginMenu("File")) {
                        if (ImGui::MenuItem("New Project...")) OnNewProject();
                        if (ImGui::MenuItem("Open Project...")) OnOpenProject();
                        
                        ImGui::Separator();
                        
                        // New Level only if none loaded
                        if (!m_LevelLoaded) {
                            if (ImGui::MenuItem("New Level")) OnNewScene();
                        }
                        
                        if (ImGui::MenuItem("Open Level...", "Cmd+O")) OnOpenScene();
                        
                        if (m_LevelLoaded) {
                            if (ImGui::MenuItem("Save Level", "Cmd+S")) OnSaveScene();
                            if (ImGui::MenuItem("Save Level As...")) OnSaveSceneAs();
                            if (ImGui::MenuItem("Close Level")) CloseScene();
                        }

                        ImGui::Separator();

                        if (ImGui::MenuItem("Exit", "Cmd+Q")) m_Running = false;

                        ImGui::EndMenu();
                    }

                    if (ImGui::BeginMenu("Settings")) {
                        if (ImGui::MenuItem("Settings")) m_ShowSettingsWindow = true;
                        if (ImGui::MenuItem("Project Settings")) { /* Dummy */ }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMainMenuBar();
                }

                m_SceneHierarchyPanel->OnImGuiRender();
                m_ContentBrowserPanel->OnImGuiRender();

                if (m_ShowSettingsWindow)
                    UI_SettingsWindow();

                // Scene Viewport
                ImGui::Begin("Scene");
                m_SceneViewportFocused = ImGui::IsWindowFocused();
                m_SceneViewportHovered = ImGui::IsWindowHovered();

                ImVec2 viewportOffset = ImGui::GetCursorScreenPos();
                m_SceneViewportPos = { viewportOffset.x, viewportOffset.y };
                if (m_SceneState != SceneState::Play)
                    m_ImGuiLayer->SetBlockEvents((!m_SceneViewportFocused || !m_SceneViewportHovered) && !ImGuizmo::IsOver());
                
                ImVec2 sceneSize = ImGui::GetContentRegionAvail();
                m_SceneViewportSize = { sceneSize.x, sceneSize.y };
                
                if (m_LevelLoaded) {
                    ImGui::Image((void*)(uint64_t)m_SceneFramebuffer->GetColorAttachmentRendererID(), sceneSize, { 0, 1 }, { 1, 0 });

                    // Drag & Drop
                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
                            const char* path = (const char*)payload->Data;
                            std::filesystem::path assetPath = path;

                            if (assetPath.extension() == ".obj" || assetPath.extension() == ".stl") {
                                Ref<VertexArray> mesh = nullptr;
                                if (assetPath.extension() == ".obj")
                                    mesh = MeshLoader::LoadOBJ(assetPath.string());
                                else if (assetPath.extension() == ".stl")
                                    mesh = MeshLoader::LoadSTL(assetPath.string());

                                if (mesh) {
                                    auto entity = CreateRef<Entity>(assetPath.stem().string(), mesh, m_DefaultShader, m_DefaultTexture);
                                    glm::vec3 dropPos = m_EditorCamera->GetPosition() + m_EditorCamera->GetForward() * 5.0f;
                                    entity->Transform.Position = dropPos;
                                    
                                    auto& bodyInterface = PhysicsSystem::GetBodyInterface();
                                    JPH::BodyCreationSettings settings(PhysicsShapes::CreateBox({ 1.0f, 1.0f, 1.0f }), JPH::RVec3(dropPos.x, dropPos.y, dropPos.z), JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::NON_MOVING);
                                    entity->PhysicsBody = bodyInterface.CreateAndAddBody(settings, JPH::EActivation::DontActivate);
                                    
                                    m_Scene->AddEntity(entity);
                                    m_SceneHierarchyPanel->SetSelectedEntity(entity);
                                }
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }

                    // Gizmos
                    Ref<Entity> selectedEntity = m_SceneHierarchyPanel->GetSelectedEntity();
                    if (selectedEntity && m_GizmoType != -1) {
                        ImGuizmo::SetOrthographic(false);
                        ImGuizmo::SetDrawlist();

                        ImGuizmo::SetRect(m_SceneViewportPos.x, m_SceneViewportPos.y, m_SceneViewportSize.x, m_SceneViewportSize.y);

                        // Editor camera
                        const glm::mat4& cameraProjection = m_EditorCamera->GetProjectionMatrix();
                        glm::mat4 cameraView = m_EditorCamera->GetViewMatrix();

                        // Entity transform
                        glm::mat4 transform = selectedEntity->Transform.GetTransform();

                        // Snapping
                        bool snap = Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL) || Input::IsKeyPressed(GLFW_KEY_LEFT_SUPER);
                        float snapValue = 0.5f; // Snap to 0.5m for translation/scale
                        if (m_GizmoType == (int)ImGuizmo::OPERATION::ROTATE)
                            snapValue = 45.0f;

                        float snapValues[3] = { snapValue, snapValue, snapValue };

                        ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                            (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
                            nullptr, snap ? snapValues : nullptr);

                        if (ImGuizmo::IsUsing()) {
                            if (!m_IsDraggingGizmo) {
                                m_IsDraggingGizmo = true;
                                m_InitialGizmoTransform = selectedEntity->Transform;
                            }

                            float translation[3], rotation[3], scale[3];
                            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), translation, rotation, scale);
                            
                            selectedEntity->Transform.Position = { translation[0], translation[1], translation[2] };
                            selectedEntity->Transform.Rotation = { rotation[0], rotation[1], rotation[2] };
                            selectedEntity->Transform.Scale = { scale[0], scale[1], scale[2] };
                        } else {
                            if (m_IsDraggingGizmo) {
                                m_IsDraggingGizmo = false;
                                m_UndoSystem.AddCommand(CreateScope<TransformCommand>(selectedEntity, m_InitialGizmoTransform, selectedEntity->Transform));
                            }
                        }
                    }
                } else {
                    std::string text = "No level open";
                    ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
                    ImGui::SetCursorPos({ (sceneSize.x - textSize.x) * 0.5f, (sceneSize.y - textSize.y) * 0.5f });
                    ImGui::Text("%s", text.c_str());
                }
                ImGui::End();

                // Game Viewport
                ImGui::Begin("Game");
                m_GameViewportFocused = ImGui::IsWindowFocused();
                m_GameViewportHovered = ImGui::IsWindowHovered();
                if (m_SceneState == SceneState::Play)
                    m_ImGuiLayer->SetBlockEvents(!m_GameViewportFocused || !m_GameViewportHovered);

                ImVec2 gameSize = ImGui::GetContentRegionAvail();
                m_GameViewportSize = { gameSize.x, gameSize.y };

                if (m_LevelLoaded) {
                    ImGui::Image((void*)(uint64_t)m_GameFramebuffer->GetColorAttachmentRendererID(), gameSize, { 0, 1 }, { 1, 0 });
                } else {
                    std::string text = "No level open";
                    ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
                    ImGui::SetCursorPos({ (gameSize.x - textSize.x) * 0.5f, (gameSize.y - textSize.y) * 0.5f });
                    ImGui::Text("%s", text.c_str());
                }
                ImGui::End();

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
                    if (ImGui::Button("Stop")) OnSceneStop();
                } else {
                    if (ImGui::Button("Pause")) OnScenePause();
                    ImGui::SameLine();
                    if (ImGui::Button("Stop")) OnSceneStop();
                }
                ImGui::SameLine();
                if (ImGui::Button("Reset")) ResetScene();
                
                ImGui::End();

                ImGui::Begin("Engine Statistics");
                float speed = m_PlayerController ? m_PlayerController->GetSpeed() : 0.0f;
                ImGui::Text("%.3f ms/frame (%.1f FPS) | Speed: %.2f units/s", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate, speed);
                ImGui::End();
            }
            m_ImGuiLayer->End();

            m_Window->OnUpdate();
        }
    }

    bool Application::OnWindowClose(WindowCloseEvent& e) {
        m_Running = false;
        return true;
    }

    bool Application::OnWindowResize(WindowResizeEvent& e) {
        if (e.GetWidth() == 0 || e.GetHeight() == 0) {
            return false;
        }

        Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());
        m_SceneFramebuffer->Resize(e.GetWidth(), e.GetHeight());
        m_GameFramebuffer->Resize(e.GetWidth(), e.GetHeight());
        m_Camera->SetProjection(45.0f, (float)e.GetWidth() / (float)e.GetHeight(), 0.1f, 100.0f);
        return false;
    }

    bool Application::OnWindowDrop(WindowDropEvent& e) {
        if (!m_LevelLoaded) {
            S67_CORE_WARN("Cannot import files without an open project/level!");
            return false;
        }

        std::filesystem::path targetDir = m_ContentBrowserPanel->GetCurrentDirectory();
        
        for (const auto& pathStr : e.GetPaths()) {
            std::filesystem::path sourcePath(pathStr);
            std::filesystem::path targetPath = targetDir / sourcePath.filename();

            try {
                if (std::filesystem::exists(targetPath)) {
                    S67_CORE_WARN("File already exists: {0}. Skipping.", targetPath.string());
                    continue;
                }

                if (std::filesystem::is_directory(sourcePath)) {
                    std::filesystem::copy(sourcePath, targetPath, std::filesystem::copy_options::recursive);
                } else {
                    std::filesystem::copy_file(sourcePath, targetPath);
                }
                S67_CORE_INFO("Imported: {0} -> {1}", sourcePath.string(), targetPath.string());
            } catch (const std::filesystem::filesystem_error& err) {
                S67_CORE_ERROR("Failed to import {0}: {1}", sourcePath.string(), err.what());
            }
        }

        return true;
    }

    void Application::UI_SettingsWindow() {
        ImGui::Begin("Settings", &m_ShowSettingsWindow);

        if (ImGui::CollapsingHeader("Appearance", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("UI Font Settings");
            if (ImGui::DragFloat("Font Scale", &m_FontSize, 0.01f, 0.5f, 2.0f, "%.2f")) {
                ImGui::GetIO().FontGlobalScale = m_FontSize / 18.0f;
            }

            ImGui::Separator();
            ImGui::Text("Themes");
            if (ImGui::Button("Unity Dark")) {
                m_EditorTheme = EditorTheme::Unity;
                m_ImGuiLayer->SetDarkThemeColors();
            }
            ImGui::SameLine();
            if (ImGui::Button("Dracula")) {
                m_EditorTheme = EditorTheme::Dracula;
                m_ImGuiLayer->SetDraculaThemeColors();
            }
            ImGui::SameLine();
            if (ImGui::Button("Classic Dark")) {
                m_EditorTheme = EditorTheme::Classic;
                ImGui::StyleColorsDark();
            }
            ImGui::SameLine();
            if (ImGui::Button("Light")) {
                m_EditorTheme = EditorTheme::Light;
                ImGui::StyleColorsLight();
            }

            ImGui::Separator();
            ImGui::Text("Custom Colors");
            if (ImGui::ColorEdit4("Window BG", glm::value_ptr(m_CustomColor))) {
                auto& colors = ImGui::GetStyle().Colors;
                colors[ImGuiCol_WindowBg] = ImVec4{ m_CustomColor.r, m_CustomColor.g, m_CustomColor.b, m_CustomColor.a };
            }
        }

        ImGui::Separator();
        if (ImGui::Button("Apply & Save")) {
            SaveSettings();
        }

        ImGui::End();
    }

    void Application::SaveSettings() {
        nlohmann::json j;
        j["FontSize"] = m_FontSize;
        j["Theme"] = (int)m_EditorTheme;
        j["CustomColor"] = { m_CustomColor.r, m_CustomColor.g, m_CustomColor.b, m_CustomColor.a };

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
                m_FontSize = j.at("FontSize").get<float>();
                m_EditorTheme = (EditorTheme)j.at("Theme").get<int>();
                
                if (j.contains("CustomColor")) {
                    m_CustomColor.r = j["CustomColor"][0];
                    m_CustomColor.g = j["CustomColor"][1];
                    m_CustomColor.b = j["CustomColor"][2];
                    m_CustomColor.a = j["CustomColor"][3];
                }

                // Apply
                ImGui::GetIO().FontGlobalScale = m_FontSize / 18.0f;
                switch (m_EditorTheme) {
                    case EditorTheme::Unity: m_ImGuiLayer->SetDarkThemeColors(); break;
                    case EditorTheme::Dracula: m_ImGuiLayer->SetDraculaThemeColors(); break;
                    case EditorTheme::Classic: ImGui::StyleColorsDark(); break;
                    case EditorTheme::Light: ImGui::StyleColorsLight(); break;
                }
                
                auto& colors = ImGui::GetStyle().Colors;
                colors[ImGuiCol_WindowBg] = ImVec4{ m_CustomColor.r, m_CustomColor.g, m_CustomColor.b, m_CustomColor.a };
                
                S67_CORE_INFO("Loaded settings from settings.json");
            } catch (...) {
                S67_CORE_ERROR("Error parsing settings.json! Using defaults.");
            }
        } else {
            // Defaults
            m_FontSize = 18.0f;
            m_EditorTheme = EditorTheme::Dracula;
            m_ImGuiLayer->SetDraculaThemeColors();
            S67_CORE_INFO("No settings.json found, using defaults (Dracula, 18px)");
        }
    }

}
