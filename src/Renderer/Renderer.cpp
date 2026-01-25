#include "Renderer.h"
#include <glad/glad.h>

namespace S67 {

    Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData();

    void Renderer::Init() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
    }

    void Renderer::OnWindowResize(uint32_t width, uint32_t height) {
        glViewport(0, 0, width, height);
    }

    void Renderer::BeginScene(const Camera& camera) {
        s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
    }

    void Renderer::EndScene() {
    }

    void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform) {
        shader->Bind();
        shader->SetMat4("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
        shader->SetMat4("u_Transform", transform);

        vertexArray->Bind();
        glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
    }

}
