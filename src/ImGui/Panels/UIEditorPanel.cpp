#include "UIEditorPanel.h"
#include "UI/UISystem.h"
#include <cstring> // for strncpy, memset
#include <imgui.h>


namespace S67 {

UIEditorPanel::UIEditorPanel() {}

void UIEditorPanel::OnImGuiRender() {
  ImGui::Begin("UI Editor");

  // Toolbar
  if (ImGui::Button("New Layout")) {
    UISystem::NewLayout();
  }
  ImGui::SameLine();
  if (ImGui::Button("Save")) {
    // Hardcoded path for now or dialog later
    UISystem::SaveLayout("assets/ui/layout.sui");
  }
  ImGui::SameLine();
  if (ImGui::Button("Load")) {
    UISystem::LoadLayout("assets/ui/layout.sui");
  }

  ImGui::Separator();

  // Left: Hierarchy/Elements List
  ImGui::BeginChild("Hierarchy", ImVec2(200, 0), true);
  DrawHierarchy();
  ImGui::EndChild();

  ImGui::SameLine();

  // Right: Inspector
  ImGui::BeginChild("Inspector", ImVec2(0, 0), true);
  DrawInspector();
  ImGui::EndChild();

  ImGui::End();
}

void UIEditorPanel::DrawHierarchy() {
  if (ImGui::Button("Add Element")) {
    ImGui::OpenPopup("AddElementPopup");
  }

  if (ImGui::BeginPopup("AddElementPopup")) {
    if (ImGui::MenuItem("Rectangle")) {
      UIElement el;
      el.Type = UIType::Rectangle;
      el.Name = "Rectangle";
      UISystem::AddElement(el);
    }
    if (ImGui::MenuItem("Text")) {
      UIElement el;
      el.Type = UIType::Text;
      el.Name = "Text Label";
      UISystem::AddElement(el);
    }
    if (ImGui::MenuItem("Circle")) {
      UIElement el;
      el.Type = UIType::Circle;
      el.Name = "Circle";
      UISystem::AddElement(el);
    }
    ImGui::EndPopup();
  }

  ImGui::Separator();

  auto &layout = UISystem::GetActiveLayout();
  for (int i = 0; i < layout.Elements.size(); i++) {
    UIElement &el = layout.Elements[i];

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    if (i == m_SelectedElementIndex)
      flags |= ImGuiTreeNodeFlags_Selected;

    std::string label = el.Name + "##" + std::to_string(i);
    ImGui::TreeNodeEx(label.c_str(), flags);
    if (ImGui::IsItemClicked()) {
      m_SelectedElementIndex = i;
      m_SelectedElement = &layout.Elements[i];
    }

    // Context menu to delete
    if (ImGui::BeginPopupContextItem()) {
      if (ImGui::MenuItem("Delete")) {
        UISystem::RemoveElement(i);
        m_SelectedElementIndex = -1;
        m_SelectedElement = nullptr;
        ImGui::EndPopup();
        break; // Break loop since we modified container
      }
      ImGui::EndPopup();
    }
  }
}

void UIEditorPanel::DrawInspector() {
  if (!m_SelectedElement) {
    ImGui::Text("Select an element to edit.");
    return;
  }

  UIElement &el = *m_SelectedElement;

  ImGui::Text("Properties");
  ImGui::Separator();

  char buffer[256];
  memset(buffer, 0, sizeof(buffer));
  std::strncpy(buffer, el.Name.c_str(), sizeof(buffer));
  if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
    el.Name = std::string(buffer);
  }

  ImGui::Checkbox("Visible", &el.Visible);

  ImGui::DragFloat2("Position", &el.Position.x);
  ImGui::DragFloat2("Size", &el.Size.x);
  ImGui::ColorEdit4("Color", &el.Color.r);

  if (el.Type == UIType::Text) {
    memset(buffer, 0, sizeof(buffer));
    std::strncpy(buffer, el.TextContent.c_str(), sizeof(buffer));
    if (ImGui::InputText("Content", buffer, sizeof(buffer))) {
      el.TextContent = std::string(buffer);
    }
    ImGui::DragFloat("Font Scale", &el.FontSize, 0.1f, 0.1f, 10.0f);
  }

  if (el.Type == UIType::Rectangle || el.Type == UIType::Square) {
    // Maybe border radius later
  }
}

} // namespace S67
