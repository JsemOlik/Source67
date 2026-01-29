#pragma once

#include "UI/UIElement.h"

namespace S67 {

class UIEditorPanel {
public:
  UIEditorPanel();
  void OnImGuiRender();

private:
  void DrawInspector();
  void DrawHierarchy();

  UIElement *m_SelectedElement = nullptr;

  // Selection state
  int m_SelectedElementIndex = -1;
};

} // namespace S67
