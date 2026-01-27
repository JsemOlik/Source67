#include "SceneHierarchyPanel.h"
#include "Core/Application.h"
#include "Core/Logger.h"
#include "Core/UndoSystem.h"
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>


namespace S67 {

SceneHierarchyPanel::SceneHierarchyPanel(const Scope<Scene> &context) {
  SetContext(context);
}

void SceneHierarchyPanel::SetContext(const Scope<Scene> &context) {
  m_Context = &context;
}

void SceneHierarchyPanel::OnImGuiRender() {
  ImGui::Begin("Scene Hierarchy");

  if (m_Context) {
    for (auto &entity : (*m_Context)->GetEntities()) {
      DrawEntityNode(entity);
    }
  }

  if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
    m_SelectionContext = nullptr;

  // Defer deletion
  if (m_EntityToDelete) {
    (*m_Context)->RemoveEntity(m_EntityToDelete);
    if (m_SelectionContext == m_EntityToDelete)
      m_SelectionContext = nullptr;
    m_EntityToDelete = nullptr;
  }

  if (m_RenamingEntity)
    ImGui::OpenPopup("Rename Entity");

  if (ImGui::BeginPopupModal("Rename Entity", NULL,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    static char buffer[256];
    if (ImGui::IsWindowAppearing() && m_RenamingEntity)
      strncpy(buffer, m_RenamingEntity->Name.c_str(), sizeof(buffer) - 1);

    if (ImGui::InputText("Name", buffer, sizeof(buffer),
                         ImGuiInputTextFlags_EnterReturnsTrue)) {
      if (m_RenamingEntity)
        m_RenamingEntity->Name = buffer;
      m_RenamingEntity = nullptr;
      ImGui::CloseCurrentPopup();
    }

    if (ImGui::Button("OK", ImVec2(120, 0))) {
      if (m_RenamingEntity)
        m_RenamingEntity->Name = buffer;
      m_RenamingEntity = nullptr;
      ImGui::CloseCurrentPopup();
    }
    ImGui::SetItemDefaultFocus();
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
      m_RenamingEntity = nullptr;
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  ImGui::End();

  ImGui::Begin("Inspector");
  if (m_SelectionContext) {
    DrawProperties(m_SelectionContext);
  }
  ImGui::End();
}

void SceneHierarchyPanel::DrawEntityNode(Ref<Entity> entity) {
  auto &name = entity->Name;

  ImGuiTreeNodeFlags flags =
      ((m_SelectionContext == entity && !m_SelectionIsMaterial)
           ? ImGuiTreeNodeFlags_Selected
           : 0) |
      ImGuiTreeNodeFlags_OpenOnArrow;
  flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
  bool opened = ImGui::TreeNodeEx((void *)(uint64_t)entity.get(), flags, "%s",
                                  name.c_str());
  if (ImGui::IsItemClicked()) {
    m_SelectionContext = entity;
    m_SelectionIsMaterial = false;
  }

  if (ImGui::BeginPopupContextItem()) {
    if (ImGui::MenuItem("Rename"))
      m_RenamingEntity = entity;

    if (ImGui::MenuItem("Delete Geometry"))
      m_EntityToDelete = entity;

    ImGui::EndPopup();
  }

  if (ImGui::BeginDragDropTarget()) {
    // ... (Drag Drop Logic) ...
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
      const char *path = (const char *)payload->Data;
      std::filesystem::path assetPath = path;

      std::string ext = assetPath.extension().string();
      std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
      bool isImage = ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
                     ext == ".bmp" || ext == ".tga";

      if (isImage) {
        auto newTexture = Texture2D::Create(assetPath.string());
        S67_CORE_INFO("Dropped texture {0} onto {1}", assetPath.string(),
                      entity->Name);
        Application::Get().GetUndoSystem().AddCommand(
            CreateScope<TextureCommand>(entity, entity->Material.AlbedoMap,
                                        newTexture));
        entity->Material.AlbedoMap = newTexture;
      }
    }
    ImGui::EndDragDropTarget();
  }

  if (opened) {
    if (entity->Material.AlbedoMap) {
      ImGuiTreeNodeFlags leafFlags =
          (m_SelectionContext == entity && m_SelectionIsMaterial)
              ? ImGuiTreeNodeFlags_Selected
              : 0;
      leafFlags |= ImGuiTreeNodeFlags_Leaf |
                   ImGuiTreeNodeFlags_NoTreePushOnOpen |
                   ImGuiTreeNodeFlags_SpanAvailWidth;

      std::string texName =
          std::filesystem::path(entity->Material.AlbedoMap->GetPath())
              .filename()
              .string();
      ImGui::TreeNodeEx((void *)((uint64_t)entity.get() + 1), leafFlags,
                        "Texture: %s", texName.c_str());
      if (ImGui::IsItemClicked()) {
        m_SelectionContext = entity;
        m_SelectionIsMaterial = true;
      }
    }
    ImGui::TreePop();
  }
}

static bool DrawVec2Control(const std::string &label, glm::vec2 &values,
                            float resetValue = 0.0f,
                            float columnWidth = 100.0f) {
  bool changed = false;
  ImGui::PushID(label.c_str());

  ImGui::Columns(2);
  ImGui::SetColumnWidth(0, columnWidth);
  ImGui::Text("%s", label.c_str());
  ImGui::NextColumn();

  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

  float lineHeight =
      ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
  ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};
  float dragFloatWidth =
      (ImGui::GetContentRegionAvail().x - 2.0f * buttonSize.x) / 2.0f;

  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.7f, 0.15f, 0.15f, 1.0f});
  if (ImGui::Button("X", buttonSize)) {
    values.x = resetValue;
    changed = true;
  }
  ImGui::PopStyleColor();

  ImGui::SameLine();
  ImGui::PushItemWidth(dragFloatWidth);
  if (ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f"))
    changed = true;
  ImGui::PopItemWidth();
  ImGui::SameLine();

  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.15f, 0.6f, 0.15f, 1.0f});
  if (ImGui::Button("Y", buttonSize)) {
    values.y = resetValue;
    changed = true;
  }
  ImGui::PopStyleColor();

  ImGui::SameLine();
  ImGui::PushItemWidth(dragFloatWidth);
  if (ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f"))
    changed = true;
  ImGui::PopItemWidth();

  ImGui::PopStyleVar();
  ImGui::Columns(1);
  ImGui::PopID();

  return changed;
}

static bool DrawVec3Control(const std::string &label, glm::vec3 &values,
                            float resetValue = 0.0f,
                            float columnWidth = 100.0f) {
  bool changed = false;
  ImGui::PushID(label.c_str());

  ImGui::Columns(2);
  ImGui::SetColumnWidth(0, columnWidth);
  ImGui::Text("%s", label.c_str());
  ImGui::NextColumn();

  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

  float lineHeight =
      ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
  ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};
  float dragFloatWidth =
      (ImGui::GetContentRegionAvail().x - 3.0f * buttonSize.x) / 3.0f;

  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.7f, 0.15f, 0.15f, 1.0f});
  if (ImGui::Button("X", buttonSize)) {
    values.x = resetValue;
    changed = true;
  }
  ImGui::PopStyleColor();

  ImGui::SameLine();
  ImGui::PushItemWidth(dragFloatWidth);
  if (ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f"))
    changed = true;
  ImGui::PopItemWidth();
  ImGui::SameLine();

  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.15f, 0.6f, 0.15f, 1.0f});
  if (ImGui::Button("Y", buttonSize)) {
    values.y = resetValue;
    changed = true;
  }
  ImGui::PopStyleColor();

  ImGui::SameLine();
  ImGui::PushItemWidth(dragFloatWidth);
  if (ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f"))
    changed = true;
  ImGui::PopItemWidth();
  ImGui::SameLine();

  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.15f, 0.15f, 0.7f, 1.0f});
  if (ImGui::Button("Z", buttonSize)) {
    values.z = resetValue;
    changed = true;
  }
  ImGui::PopStyleColor();

  ImGui::SameLine();
  ImGui::PushItemWidth(dragFloatWidth);
  if (ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f"))
    changed = true;
  ImGui::PopItemWidth();

  ImGui::PopStyleVar();
  ImGui::Columns(1);
  ImGui::PopID();

  return changed || ImGui::IsItemDeactivatedAfterEdit();
}

void SceneHierarchyPanel::DrawProperties(Ref<Entity> entity) {
  if (m_SelectionIsMaterial) {
    if (ImGui::CollapsingHeader("Material Properties",
                                ImGuiTreeNodeFlags_DefaultOpen)) {
      if (entity->Material.AlbedoMap) {
        ImGui::Text("Texture: %s",
                    std::filesystem::path(entity->Material.AlbedoMap->GetPath())
                        .filename()
                        .string()
                        .c_str());
        DrawVec2Control("Tiling", entity->Material.Tiling, 1.0f);
        ImGui::Spacing();
      }
    }
  } else {
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
      Transform oldTransform = entity->Transform;
      bool changed = false;

      if (DrawVec3Control("Position", entity->Transform.Position))
        changed = true;

      glm::vec3 rotation = entity->Transform.Rotation;
      if (DrawVec3Control("Rotation", rotation)) {
        entity->Transform.Rotation = rotation;
        changed = true;
      }

      if (DrawVec3Control("Scale", entity->Transform.Scale, 1.0f))
        changed = true;

      if (changed && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        Application::Get().GetUndoSystem().AddCommand(
            CreateScope<TransformCommand>(entity, oldTransform,
                                          entity->Transform));
      }

      if (ImGui::Checkbox("Collidable", &entity->Collidable)) {
        Application::Get().OnEntityCollidableChanged(entity);
      }
      ImGui::Spacing();
    }

    if (entity->Material.AlbedoMap) {
      if (ImGui::CollapsingHeader("Material Properties",
                                  ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Texture: %s",
                    std::filesystem::path(entity->Material.AlbedoMap->GetPath())
                        .filename()
                        .string()
                        .c_str());
        DrawVec2Control("Tiling", entity->Material.Tiling, 1.0f);
        ImGui::Spacing();
      }
    }
  }
}

} // namespace S67
