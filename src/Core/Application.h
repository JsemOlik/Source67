#include "Base.h"
#include "Window.h"
#include "Events/WindowEvent.h"
#include "Renderer/Shader.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Texture.h"
#include "Renderer/Camera.h"
#include "Renderer/Scene.h"
#include "Renderer/Light.h"
#include "Renderer/CameraController.h"
#include "Physics/PhysicsSystem.h"
#include "Renderer/Framebuffer.h"
#include "ImGui/ImGuiLayer.h"
#include "ImGui/Panels/SceneHierarchyPanel.h"
#include "Core/UndoSystem.h"
#include "Physics/PlayerController.h"
#include <filesystem>

namespace S67 {

    class ContentBrowserPanel;

    enum class SceneState {
        Edit = 0, Play = 1, Pause = 2
    };

    class Application {
    public:
        Application();
        virtual ~Application();

        void Run();

        void OnEvent(Event& e);

        void OnScenePlay();
        void OnScenePause();
        void OnSceneStop();

        void OnSaveScene(); // Quick Save (Shortcuts)
        void OnSaveSceneAs(); // Dialog version
        void OnOpenScene(); // Dialog version
        void OpenScene(const std::string& filepath); // Direct version
        void OnNewScene();
        void CloseScene();

        void OnEntityCollidableChanged(Ref<Entity> entity);

        void OnNewProject();
        void OnOpenProject();
        void DiscoverProject(const std::filesystem::path& levelPath);

        void CreateTestScene();
        void ResetScene();

        inline Window& GetWindow() { return *m_Window; }
        ImGuiLayer& GetImGuiLayer() { return *m_ImGuiLayer; }
        inline static Application& Get() { return *s_Instance; }

        const std::filesystem::path& GetProjectRoot() const { return m_ProjectRoot; }
        void SetProjectRoot(const std::filesystem::path& root);

        UndoSystem& GetUndoSystem() { return m_UndoSystem; }

    private:
        bool OnWindowClose(WindowCloseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);

        std::unique_ptr<Window> m_Window;
        bool m_Running = true;

        Ref<PerspectiveCamera> m_Camera; // Game Camera
        Ref<PerspectiveCamera> m_EditorCamera;
        Ref<CameraController> m_CameraController; // Game Camera Controller
        Ref<CameraController> m_EditorCameraController;
        Scope<Scene> m_Scene;
        DirectionalLight m_Sun;

        float m_LastFrameTime = 0.0f;
        
        Scope<PlayerController> m_PlayerController;

        Scope<ImGuiLayer> m_ImGuiLayer;
        Scope<SceneHierarchyPanel> m_SceneHierarchyPanel;
        Scope<ContentBrowserPanel> m_ContentBrowserPanel;

        std::filesystem::path m_ProjectRoot;
        std::filesystem::path m_ProjectFilePath;
        std::string m_ProjectName = "Standalone";
        std::string m_ProjectVersion = "N/A";
        bool m_LevelLoaded = false;
        std::string m_LevelFilePath = "";

        int m_GizmoType = 7; // ImGuizmo::TRANSLATE
        UndoSystem m_UndoSystem;
        Transform m_InitialGizmoTransform;
        bool m_IsDraggingGizmo = false;

        Ref<Framebuffer> m_SceneFramebuffer;
        Ref<Framebuffer> m_GameFramebuffer;
        Ref<Shader> m_OutlineShader;

        glm::vec2 m_SceneViewportSize = { 0, 0 };
        glm::vec2 m_GameViewportSize = { 0, 0 };

        glm::vec2 m_SceneViewportPos = { 0, 0 };

        bool m_SceneViewportFocused = false, m_SceneViewportHovered = false;
        bool m_GameViewportFocused = false, m_GameViewportHovered = false;

        Ref<Shader> m_DefaultShader;
        Ref<Texture2D> m_DefaultTexture;

        SceneState m_SceneState = SceneState::Edit;

        static Application* s_Instance;
    };

}
