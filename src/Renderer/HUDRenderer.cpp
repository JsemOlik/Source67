#include "HUDRenderer.h"
#include "Core/Logger.h"
#include "Renderer/Buffer.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace S67 {

HUDRenderer::HUDData *HUDRenderer::s_Data = nullptr;

void HUDRenderer::Init() {
  s_Data = new HUDData();

  // Create quad vertex array for rendering UI elements
  // Simple 2D quad: position only (x, y)
  float quadVertices[] = {
      // positions
      0.0f, 0.0f, // bottom-left
      1.0f, 0.0f, // bottom-right
      1.0f, 1.0f, // top-right
      0.0f, 1.0f  // top-left
  };

  uint32_t quadIndices[] = {
      0, 1, 2, // first triangle
      2, 3, 0  // second triangle
  };

  s_Data->QuadVAO = VertexArray::Create();

  Ref<VertexBuffer> quadVBO =
      VertexBuffer::Create(quadVertices, sizeof(quadVertices));
  quadVBO->SetLayout({{ShaderDataType::Float2, "a_Position"}});
  s_Data->QuadVAO->AddVertexBuffer(quadVBO);

  Ref<IndexBuffer> quadIBO =
      IndexBuffer::Create(quadIndices, sizeof(quadIndices) / sizeof(uint32_t));
  s_Data->QuadVAO->SetIndexBuffer(quadIBO);

  S67_CORE_INFO("HUDRenderer initialized");
}

void HUDRenderer::SetShader(const Ref<Shader> &shader) {
  if (s_Data) {
    s_Data->HUDShader = shader;
  }
}

void HUDRenderer::Shutdown() {
  if (s_Data) {
    delete s_Data;
    s_Data = nullptr;
  }
  S67_CORE_INFO("HUDRenderer shutdown");
}

void HUDRenderer::BeginHUD(float width, float height) {
  if (!s_Data)
    return;

  s_Data->ViewportWidth = width;
  s_Data->ViewportHeight = height;

  // Create orthographic projection matrix
  // Origin at bottom-left, X right, Y up
  s_Data->ProjectionMatrix = glm::ortho(0.0f, width, 0.0f, height, -1.0f, 1.0f);

  // Disable depth testing for HUD rendering
  glDisable(GL_DEPTH_TEST);

  // Enable blending for transparency
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void HUDRenderer::EndHUD() {
  // Re-enable depth testing
  glEnable(GL_DEPTH_TEST);
}

void HUDRenderer::RenderCrosshair() {
  if (!s_Data || !s_Data->HUDShader || !s_Data->HUDShader->IsValid())
    return;

  float centerX = s_Data->ViewportWidth / 2.0f;
  float centerY = s_Data->ViewportHeight / 2.0f;

  // Crosshair dimensions
  float lineLength = 20.0f;
  float lineThickness = 2.0f;
  float gap = 4.0f; // Gap in the center

  // White color with slight transparency
  glm::vec4 crosshairColor = glm::vec4(1.0f, 1.0f, 1.0f, 0.8f);

  // Render horizontal line (left part)
  RenderQuad(
      glm::vec2(centerX - lineLength - gap, centerY - lineThickness / 2.0f),
      glm::vec2(lineLength, lineThickness), crosshairColor);

  // Render horizontal line (right part)
  RenderQuad(glm::vec2(centerX + gap, centerY - lineThickness / 2.0f),
             glm::vec2(lineLength, lineThickness), crosshairColor);

  // Render vertical line (top part)
  RenderQuad(glm::vec2(centerX - lineThickness / 2.0f, centerY + gap),
             glm::vec2(lineThickness, lineLength), crosshairColor);

  // Render vertical line (bottom part)
  RenderQuad(
      glm::vec2(centerX - lineThickness / 2.0f, centerY - lineLength - gap),
      glm::vec2(lineThickness, lineLength), crosshairColor);
}

void HUDRenderer::RenderQuad(const glm::vec2 &position, const glm::vec2 &size,
                             const glm::vec4 &color) {
  if (!s_Data || !s_Data->HUDShader || !s_Data->QuadVAO)
    return;

  // Create transform matrix: translate to position, then scale to size
  glm::mat4 transform =
      glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f));
  transform = glm::scale(transform, glm::vec3(size, 1.0f));

  // Bind shader and set uniforms
  s_Data->HUDShader->Bind();
  s_Data->HUDShader->SetMat4("u_Projection", s_Data->ProjectionMatrix);
  s_Data->HUDShader->SetMat4("u_Transform", transform);
  s_Data->HUDShader->SetFloat4("u_Color", color);

  // Draw quad
  s_Data->QuadVAO->Bind();
  glDrawElements(GL_TRIANGLES, s_Data->QuadVAO->GetIndexBuffer()->GetCount(),
                 GL_UNSIGNED_INT, nullptr);
}

} // namespace S67
