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
#include "ImGui/ImGuiLayer.h"
#include "ImGui/Panels/SceneHierarchyPanel.h"

namespace S67 {

    enum class SceneState {
        Edit = 0, Play = 1
    };

    class Application {
    public:
        Application();
        virtual ~Application();

        void Run();

        void OnEvent(Event& e);

        void OnScenePlay();
        void OnSceneStop();

        inline Window& GetWindow() { return *m_Window; }
        ImGuiLayer& GetImGuiLayer() { return *m_ImGuiLayer; }
        inline static Application& Get() { return *s_Instance; }

    private:
        bool OnWindowClose(WindowCloseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);

        std::unique_ptr<Window> m_Window;
        bool m_Running = true;

        Ref<PerspectiveCamera> m_Camera;
        Ref<CameraController> m_CameraController;
        Scope<Scene> m_Scene;
        DirectionalLight m_Sun;

        float m_LastFrameTime = 0.0f;

        Scope<ImGuiLayer> m_ImGuiLayer;
        Scope<SceneHierarchyPanel> m_SceneHierarchyPanel;

        SceneState m_SceneState = SceneState::Edit;

        static Application* s_Instance;
    };

}
