#include "Base.h"
#include "Window.h"
#include "Events/WindowEvent.h"
#include "Renderer/Shader.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Texture.h"
#include "Renderer/Camera.h"

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

        Ref<Shader> m_Shader;
        Ref<VertexArray> m_VertexArray;
        Ref<Texture2D> m_Texture;
        Ref<PerspectiveCamera> m_Camera;

        static Application* s_Instance;
    };

}
