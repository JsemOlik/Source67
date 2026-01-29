#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>


namespace S67 {

enum class UIType { Rectangle = 0, Square, Circle, Text };

struct UIElement {
  UIType Type = UIType::Rectangle;
  std::string Name = "New Element";
  glm::vec2 Position = {100.0f, 100.0f};
  glm::vec2 Size = {100.0f, 100.0f};
  glm::vec4 Color = {1.0f, 1.0f, 1.0f, 1.0f};

  // Text specific
  std::string TextContent = "Text";
  float FontSize = 1.0f;

  bool Visible = true;
};

struct UILayout {
  std::string Name = "New Layout";
  std::vector<UIElement> Elements;
};

} // namespace S67
