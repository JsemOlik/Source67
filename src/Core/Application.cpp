#include "Application.h"
#include "Logger.h"
#include "Renderer/Renderer.h"
#include "Renderer/Shader.h"
#include "Renderer/Buffer.h"
#include "Renderer/VertexArray.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace S67 {

    Application* Application::s_Instance = nullptr;

    Application::Application() {
        S67_CORE_ASSERT(!s_Instance, "Application already exists!");
        s_Instance = this;

        S67_CORE_INFO("Initializing Window...");
        m_Window = std::unique_ptr<Window>(Window::Create());
        m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

        S67_CORE_INFO("Initializing Renderer...");
        Renderer::Init();

        // Testing Quad
        S67_CORE_INFO("Setting up test quad...");
        m_VertexArray = Ref<VertexArray>(VertexArray::Create());

        float vertices[4 * 5] = {
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
             0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
             0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
            -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
        };

        Ref<VertexBuffer> vertexBuffer;
        vertexBuffer = Ref<VertexBuffer>(VertexBuffer::Create(vertices, sizeof(vertices)));
        BufferLayout layout = {
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float2, "a_TexCoord" }
        };
        vertexBuffer->SetLayout(layout);
        m_VertexArray->AddVertexBuffer(vertexBuffer);

        uint32_t indices[6] = { 0, 1, 2, 2, 3, 0 };
        Ref<IndexBuffer> indexBuffer;
        indexBuffer = Ref<IndexBuffer>(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
        m_VertexArray->SetIndexBuffer(indexBuffer);

        S67_CORE_INFO("Loading shaders and textures...");
        m_Shader = Shader::Create("assets/shaders/Texture.glsl");
        m_Texture = Texture2D::Create("assets/textures/Checkerboard.png");

        m_Shader->Bind();
        m_Shader->SetInt("u_Texture", 0);

        S67_CORE_INFO("Application initialized successfully");
    }

    Application::~Application() {
    }

    void Application::OnEvent(Event& e) {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));

        S67_CORE_TRACE("{0}", e.ToString());
    }

    void Application::Run() {
        while (m_Running) {
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            Renderer::BeginScene();

            m_Shader->Bind();
            m_Texture->Bind();
            Renderer::Submit(m_VertexArray);

            Renderer::EndScene();

            m_Window->OnUpdate();
        }
    }

    bool Application::OnWindowClose(WindowCloseEvent& e) {
        m_Running = false;
        return true;
    }

    bool Application::OnWindowResize(WindowResizeEvent& e) {
        Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());
        return false;
    }

}
