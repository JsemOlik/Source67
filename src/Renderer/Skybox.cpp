#include "Skybox.h"
#include "Core/Application.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace S67 {

Skybox::Skybox(const std::string &texturePath) {
  m_Shader = Shader::Create(Application::Get()
                                .ResolveAssetPath("assets/shaders/Skybox.glsl")
                                .string());
  m_Texture = Texture2D::Create(texturePath);

  float vertices[] = {// Back face
                      -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
                      -1.0f, -1.0f, 1.0f, -1.0f,
                      // Front face
                      -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                      -1.0f, 1.0f, 1.0f,
                      // Left face
                      -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f,
                      1.0f, -1.0f, 1.0f, -1.0f,
                      // Right face
                      1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                      1.0f, 1.0f, -1.0f,
                      // Bottom face
                      -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
                      1.0f, -1.0f, -1.0f, 1.0f,
                      // Top face
                      -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
                      -1.0f, 1.0f, -1.0f};

  m_VertexArray = VertexArray::Create();
  auto vbo = VertexBuffer::Create(vertices, sizeof(vertices));
  vbo->SetLayout({{ShaderDataType::Float3, "a_Position"}});
  m_VertexArray->AddVertexBuffer(vbo);

  uint32_t indices[] = {0,  1,  2,  2,  3,  0,  4,  5,  6,  6,  7,  4,
                        8,  9,  10, 10, 11, 8,  12, 13, 14, 14, 15, 12,
                        16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20};
  auto ibo = IndexBuffer::Create(indices, 36);
  m_VertexArray->SetIndexBuffer(ibo);
}

void Skybox::Draw(const Camera &camera) {
  glDepthFunc(GL_LEQUAL);
  m_Shader->Bind();
  // Remove translation from view matrix for skybox
  glm::mat4 view = camera.GetViewMatrix();
  view[3] = glm::vec4(0, 0, 0, 1);
  m_Shader->SetMat4("u_ViewProjection", camera.GetProjectionMatrix() * view);
  m_Shader->SetMat4("u_Transform", glm::mat4(1.0f));
  m_Texture->Bind(0);
  m_Shader->SetInt("u_SkyboxTexture", 0);

  m_VertexArray->Bind();
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
  glDepthFunc(GL_LESS);
}

} // namespace S67
