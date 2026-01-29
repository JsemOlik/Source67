#pragma once

#include "Core/Base.h"
#include "Renderer/Shader.h"
#include "Renderer/VertexArray.h"
#include <glm/glm.hpp>

namespace S67 {

class HUDRenderer {
public:
  static void Init();
  static void SetShader(const Ref<Shader> &shader);
  static void Shutdown();

  static void BeginHUD(float width, float height);
  static void EndHUD();

  static void RenderCrosshair();

private:
  static void RenderQuad(const glm::vec2 &position, const glm::vec2 &size,
                         const glm::vec4 &color);

  struct HUDData {
    Ref<VertexArray> QuadVAO;
    Ref<Shader> HUDShader;
    glm::mat4 ProjectionMatrix;
    float ViewportWidth;
    float ViewportHeight;
  };

  static HUDData *s_Data;
};

} // namespace S67
