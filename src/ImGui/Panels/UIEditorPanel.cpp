#include "UIEditorPanel.h"
#include "Core/Application.h"
#include "Renderer/Scene.h"
#include "UI/UISystem.h"
#include <cstring> // for strncpy, memset
#include <filesystem>
#include <imgui.h>
#include <string>

namespace S67 {

UIEditorPanel::UIEditorPanel() {}

void UIEditorPanel::OnImGuiRender() {
  ImGui::Begin("UI Editor");

  auto &app = Application::Get();
  auto *activeScene = app.GetActiveScene();

  if (activeScene) {
    char pathBuf[256];
    memset(pathBuf, 0, sizeof(pathBuf));
    std::strncpy(pathBuf, activeScene->GetUIPath().c_str(), sizeof(pathBuf));

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Linked UI");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-60.0f);
    if (ImGui::InputText("##SceneUIPath", pathBuf, sizeof(pathBuf))) {
      activeScene->SetUIPath(std::string(pathBuf));
      app.SetSceneModified(true);
    }
    ImGui::SameLine();
    if (ImGui::Button("Sync")) {
      std::string path = activeScene->GetUIPath();
      if (!path.empty() && path != "None") {
        UISystem::LoadLayout(app.ResolveAssetPath(path));
        m_SelectedElementIndex = -1;
      }
    }
    ImGui::Separator();
  }

  // Toolbar
  if (ImGui::Button("New Layout")) {
    UISystem::NewLayout();
    m_SelectedElementIndex = -1;
  }
  ImGui::SameLine();
  if (ImGui::Button("Save")) {
    std::string path =
        activeScene ? activeScene->GetUIPath() : "assets/ui/layout.sui";
    if (path.empty() || path == "None")
      path = "assets/ui/layout.sui";
    UISystem::SaveLayout(app.ResolveAssetPath(path));
  }
  ImGui::SameLine();
  if (ImGui::Button("Load")) {
    std::string path =
        activeScene ? activeScene->GetUIPath() : "assets/ui/layout.sui";
    if (path.empty() || path == "None")
      path = "assets/ui/layout.sui";
    UISystem::LoadLayout(app.ResolveAssetPath(path));
    m_SelectedElementIndex = -1;
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
    }

    // Context menu to delete
    if (ImGui::BeginPopupContextItem()) {
      if (ImGui::MenuItem("Delete")) {
        UISystem::RemoveElement(i);
        m_SelectedElementIndex = -1;
        ImGui::EndPopup();
        break; // Break loop since we modified container
      }
      ImGui::EndPopup();
    }
  }
}

void UIEditorPanel::DrawInspector() {
  auto &layout = UISystem::GetActiveLayout();
  if (m_SelectedElementIndex < 0 ||
      m_SelectedElementIndex >= layout.Elements.size()) {
    ImGui::Text("Select an element to edit.");
    return;
  }

  UIElement &el = layout.Elements[m_SelectedElementIndex];

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
