#include "UISystem.h"
#include "Core/Logger.h"
#include "Renderer/HUDRenderer.h"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>

// Allow serialization of glm types
namespace glm {
void to_json(nlohmann::json &j, const glm::vec2 &v) {
  j = nlohmann::json{{"x", v.x}, {"y", v.y}};
}
void from_json(const nlohmann::json &j, glm::vec2 &v) {
  j.at("x").get_to(v.x);
  j.at("y").get_to(v.y);
}
void to_json(nlohmann::json &j, const glm::vec4 &v) {
  j = nlohmann::json{{"r", v.r}, {"g", v.g}, {"b", v.b}, {"a", v.a}};
}
void from_json(const nlohmann::json &j, glm::vec4 &v) {
  j.at("r").get_to(v.r);
  j.at("g").get_to(v.g);
  j.at("b").get_to(v.b);
  j.at("a").get_to(v.a);
}
} // namespace glm

namespace S67 {

UILayout UISystem::s_ActiveLayout;

void UISystem::Init() {
  S67_CORE_INFO("UISystem Initialized");
  NewLayout();
}

void UISystem::Shutdown() {
  // Cleanup if needed
}

void UISystem::Render() {
  for (const auto &element : s_ActiveLayout.Elements) {
    if (!element.Visible)
      continue;
    RenderElement(element);
  }
}

// Access private HUDRenderer::RenderQuad via a friend or just expose it?
// Since we cannot modify HUDRenderer.h easily to friendship without recompile,
// We will check HUDRenderer.h. Wait, RenderQuad is private.
// We must expose RenderQuad in HUDRenderer to use it here.
// Or we can replicate the logic if we have access to VAO/Shader (private
// strut). Ideally, we expose a public API in HUDRenderer: RenderRect,
// RenderText. HUDRenderer only has RenderCrosshair, RenderSpeed, DrawString.
// DrawString IS public.
// But RenderQuad is private.
// I will modify HUDRenderer.h to make RenderQuad public (or add RenderRect
// wrapper).

void UISystem::RenderElement(const UIElement &element) {
  if (element.Type == UIType::Text) {
    // Draw String
    // HUDRenderer::DrawString expects scale.
    HUDRenderer::DrawString(element.TextContent, element.Position,
                            element.FontSize, element.Color);
  } else {
    // Shapes
    HUDRenderer::RenderRect(element.Position, element.Size, element.Color);
  }
}

void UISystem::NewLayout() { s_ActiveLayout = UILayout(); }

void UISystem::LoadLayout(const std::filesystem::path &path) {
  std::ifstream i(path);
  if (!i.is_open()) {
    S67_CORE_ERROR("Failed to load UI layout: {0}", path.string());
    return;
  }

  nlohmann::json j;
  i >> j;

  s_ActiveLayout.Name = j["Name"];
  s_ActiveLayout.Elements.clear();

  for (auto &elJson : j["Elements"]) {
    UIElement el;
    el.Type = static_cast<UIType>(elJson["Type"]);
    el.Name = elJson["Name"];
    el.Position = elJson["Position"];
    el.Size = elJson["Size"];
    el.Color = elJson["Color"];
    el.Visible = elJson["Visible"];

    if (el.Type == UIType::Text) {
      el.TextContent = elJson["TextContent"];
      el.FontSize = elJson["FontSize"];
    }

    s_ActiveLayout.Elements.push_back(el);
  }

  S67_CORE_INFO("Loaded UI Layout: {0}", s_ActiveLayout.Name);
}

void UISystem::SaveLayout(const std::filesystem::path &path) {
  nlohmann::json j;
  j["Name"] = s_ActiveLayout.Name;

  nlohmann::json els = nlohmann::json::array();

  for (const auto &el : s_ActiveLayout.Elements) {
    nlohmann::json elJson;
    elJson["Type"] = static_cast<int>(el.Type);
    elJson["Name"] = el.Name;
    elJson["Position"] = el.Position;
    elJson["Size"] = el.Size;
    elJson["Color"] = el.Color;
    elJson["Visible"] = el.Visible;

    if (el.Type == UIType::Text) {
      elJson["TextContent"] = el.TextContent;
      elJson["FontSize"] = el.FontSize;
    }
    els.push_back(elJson);
  }
  j["Elements"] = els;

  if (!path.parent_path().empty() &&
      !std::filesystem::exists(path.parent_path()))
    std::filesystem::create_directories(path.parent_path());

  std::ofstream o(path);
  o << std::setw(4) << j << std::endl;
  S67_CORE_INFO("Saved UI Layout to {0}", path.string());
}

void UISystem::AddElement(const UIElement &element) {
  s_ActiveLayout.Elements.push_back(element);
}

void UISystem::RemoveElement(int index) {
  if (index >= 0 && index < s_ActiveLayout.Elements.size()) {
    s_ActiveLayout.Elements.erase(s_ActiveLayout.Elements.begin() + index);
  }
}

} // namespace S67
