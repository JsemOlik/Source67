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

namespace S67 {

    class Application {
    public:
        Application();
        virtual ~Application();

        void Run();

        void OnEvent(Event& e);

        inline Window& GetWindow() { return *m_Window; }
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

        static Application* s_Instance;
    };

}
