#include "Application.h"
#include "Logger.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace S67 {

    Application* Application::s_Instance = nullptr;

    Application::Application() {
        S67_CORE_ASSERT(!s_Instance, "Application already exists!");
        s_Instance = this;

        m_Window = std::unique_ptr<Window>(Window::Create());
        m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
    }

    Application::~Application() {
    }

    void Application::OnEvent(Event& e) {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));

        S67_CORE_TRACE("{0}", e.ToString());
    }

    void Application::Run() {
        while (m_Running) {
            glClearColor(0.2f, 0.3f, 0.8f, 1.0f); // Source-like blueish clear color
            glClear(GL_COLOR_BUFFER_BIT);

            m_Window->OnUpdate();
        }
    }

    bool Application::OnWindowClose(WindowCloseEvent& e) {
        m_Running = false;
        return true;
    }

}
