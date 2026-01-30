#pragma once

#include "Core/Base.h"
#include "Renderer/Shader.h"
#include "Renderer/VertexArray.h"
#include <cstdint>
#include <glm/glm.hpp>
#include <string>


namespace S67 {

class HUDRenderer {
public:
  static void Init();
  static void SetShader(const Ref<Shader> &shader);
  static void Shutdown();

  static void BeginHUD(float width, float height);
  static void EndHUD();

  static void RenderCrosshair();
  static void RenderSpeed(float speed);

  static void QueueString(const std::string &text,
                          const glm::vec4 &color = {1.0f, 1.0f, 1.0f, 1.0f});

  static void SetText(const std::string &id, const std::string &text,
                      const glm::vec2 &position = {0.5f, 0.1f},
                      float scale = 3.0f,
                      const glm::vec4 &color = {1.0f, 1.0f, 1.0f, 1.0f});
  static void ClearText(const std::string &id);

  static void DrawString(const std::string &text, const glm::vec2 &position,
                         float scale, const glm::vec4 &color);

private:
  static void RenderQuad(const glm::vec2 &position, const glm::vec2 &size,
                         const glm::vec4 &color, uint32_t textureID = 0);

  struct HUDData {
    Ref<VertexArray> QuadVAO;
    Ref<Shader> HUDShader;
    uint32_t FontTextureID = 0;
    glm::mat4 ProjectionMatrix;
    float ViewportWidth;
    float ViewportHeight;

    struct QueuedText {
      std::string Text;
      glm::vec4 Color;
    };
    std::vector<QueuedText> TextQueue;

    struct PersistentText {
      std::string Text;
      glm::vec2 Position;
      float Scale;
      glm::vec4 Color;
    };
    std::map<std::string, PersistentText> PersistentTexts;
  };

  static HUDData *s_Data;
};

} // namespace S67
