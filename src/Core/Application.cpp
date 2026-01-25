#include "Application.h"
#include "Logger.h"
#include "Renderer/Renderer.h"
#include "Renderer/Shader.h"
#include "Renderer/Buffer.h"
#include "Renderer/VertexArray.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <imgui.h>
#include "Physics/PhysicsShapes.h"

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

        S67_CORE_INFO("Initializing Physics...");
        PhysicsSystem::Init();

        m_Camera = CreateRef<PerspectiveCamera>(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
        m_Camera->SetPosition({ 0.0f, 2.0f, 8.0f });

        m_Scene = CreateScope<Scene>();
        m_Sun.Direction = { -0.5f, -1.0f, -0.2f };
        m_Sun.Color = { 1.0f, 0.95f, 0.8f }; // Warm sun color
        m_Sun.Intensity = 1.0f;

        // Testing Cube with Normals
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

        auto& bodyInterface = PhysicsSystem::GetBodyInterface();

        // 1. Static Floor
        auto floorVA = vertexArray; // Reusing cube for floor
        auto floor = CreateRef<Entity>("Static Floor", floorVA, shader, texture);
        floor->Transform.Position = { 0.0f, -2.0f, 0.0f };
        floor->Transform.Scale = { 20.0f, 1.0f, 20.0f };
        
        JPH::BodyCreationSettings floorSettings(PhysicsShapes::CreateBox({ 20.0f, 1.0f, 20.0f }), JPH::RVec3(0, -2, 0), JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::NON_MOVING);
        floor->PhysicsBody = bodyInterface.CreateAndAddBody(floorSettings, JPH::EActivation::DontActivate);
        m_Scene->AddEntity(floor);

        // 2. Dynamic Cubes
        for (int i = 0; i < 5; i++) {
            std::string name = "Cube " + std::to_string(i);
            auto cube = CreateRef<Entity>(name, vertexArray, shader, texture);
            cube->Transform.Position = { (float)i * 2.0f - 4.0f, 10.0f + (float)i * 2.0f, 0.0f };
            
            JPH::BodyCreationSettings cubeSettings(PhysicsShapes::CreateBox({ 1.0f, 1.0f, 1.0f }), JPH::RVec3(cube->Transform.Position.x, cube->Transform.Position.y, cube->Transform.Position.z), JPH::Quat::sIdentity(), JPH::EMotionType::Dynamic, Layers::MOVING);
            cube->PhysicsBody = bodyInterface.CreateAndAddBody(cubeSettings, JPH::EActivation::Activate);
            m_Scene->AddEntity(cube);
        }

        m_CameraController = CreateRef<CameraController>(m_Camera);
        m_Window->SetCursorLocked(true);

        m_ImGuiLayer = CreateScope<ImGuiLayer>();
        m_ImGuiLayer->OnAttach();

        m_SceneHierarchyPanel = CreateScope<SceneHierarchyPanel>(m_Scene);

        S67_CORE_INFO("Application initialized successfully");
    }

    Application::~Application() {
        m_ImGuiLayer->OnDetach();
        PhysicsSystem::Shutdown();
    }

    void Application::OnEvent(Event& e) {
        m_ImGuiLayer->OnEvent(e);
        m_CameraController->OnEvent(e);

        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));

        S67_CORE_TRACE("{0}", e.ToString());
    }

    void Application::Run() {
        while (m_Running) {
            float time = (float)glfwGetTime();
            Timestep timestep = time - m_LastFrameTime;
            m_LastFrameTime = time;

            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            m_CameraController->OnUpdate(timestep);
            PhysicsSystem::OnUpdate(timestep);

            Renderer::BeginScene(*m_Camera, m_Sun);

            auto& bodyInterface = PhysicsSystem::GetBodyInterface();

            for (auto& entity : m_Scene->GetEntities()) {
                if (!entity->PhysicsBody.IsInvalid()) {
                    JPH::RVec3 position;
                    JPH::Quat rotation;
                    bodyInterface.GetPositionAndRotation(entity->PhysicsBody, position, rotation);

                    entity->Transform.Position = { position.GetX(), position.GetY(), position.GetZ() };
                    // Convert Quat to Euler for our simple Transform struct
                    glm::quat q = { rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ() };
                    entity->Transform.Rotation = glm::degrees(glm::eulerAngles(q));
                }

                entity->MaterialTexture->Bind();
                Renderer::Submit(entity->MaterialShader, entity->Mesh, entity->Transform.GetTransform());
            }

            Renderer::EndScene();

            m_ImGuiLayer->Begin();
            {
                m_SceneHierarchyPanel->OnImGuiRender();

                ImGui::Begin("Engine Statistics");
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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
        Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());
        m_Camera->SetProjection(45.0f, (float)e.GetWidth() / (float)e.GetHeight(), 0.1f, 100.0f);
        return false;
    }

}
