#pragma once

#include "UIElement.h"
#include <filesystem>
#include <string>
#include <vector>


namespace S67 {

class UISystem {
public:
  static void Init();
  static void Shutdown();

  // Rendering
  static void Render();

  // Management
  static void NewLayout();
  static void LoadLayout(const std::filesystem::path &path);
  static void SaveLayout(const std::filesystem::path &path);

  static UILayout &GetActiveLayout() { return s_ActiveLayout; }

  // Modification
  static void AddElement(const UIElement &element);
  static void RemoveElement(int index);

private:
  static UILayout s_ActiveLayout;
  // Helper to render specific shapes
  static void RenderElement(const UIElement &element);
};

} // namespace S67
