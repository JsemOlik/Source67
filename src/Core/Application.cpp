#include "Application.h"
#include "Logger.h"
#include "Renderer/Renderer.h"
#include "Renderer/Shader.h"
#include "Renderer/Buffer.h"
#include "Renderer/VertexArray.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

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

        m_Camera = CreateRef<PerspectiveCamera>(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
        m_Camera->SetPosition({ 0.0f, 0.0f, 5.0f });

        // Testing Cube
        S67_CORE_INFO("Setting up test cube...");
        m_VertexArray = VertexArray::Create();

        float vertices[] = {
            // Front
            -1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
            -1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
            // Back
            -1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
             1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f,  1.0f, -1.0f, 0.0f, 1.0f,
            -1.0f,  1.0f, -1.0f, 1.0f, 1.0f,
            // Top
            -1.0f,  1.0f,  1.0f, 0.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f, 0.0f,
             1.0f,  1.0f, -1.0f, 1.0f, 1.0f,
            -1.0f,  1.0f, -1.0f, 0.0f, 1.0f,
            // Bottom
            -1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, -1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
            // Left
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
            -1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
            -1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
            -1.0f,  1.0f, -1.0f, 0.0f, 1.0f,
            // Right
             1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
             1.0f,  1.0f, -1.0f, 1.0f, 1.0f
        };

        Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
        vertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float2, "a_TexCoord" }
        });
        m_VertexArray->AddVertexBuffer(vertexBuffer);

        uint32_t indices[] = {
            0,  1,  2,  2,  3,  0,
            4,  5,  6,  6,  7,  4,
            8,  9,  10, 10, 11, 8,
            12, 13, 14, 14, 15, 12,
            16, 17, 18, 18, 19, 16,
            20, 21, 22, 22, 23, 20
        };
        m_VertexArray->SetIndexBuffer(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));

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

            static float rotation = 0.0f;
            rotation += 1.0f; // Simple increment for testing

            Renderer::BeginScene(*m_Camera);

            glm::mat4 transform = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0.5f, 1.0f, 0.0f));
            m_Texture->Bind();
            Renderer::Submit(m_Shader, m_VertexArray, transform);

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
        m_Camera->SetProjection(45.0f, (float)e.GetWidth() / (float)e.GetHeight(), 0.1f, 100.0f);
        return false;
    }

}
