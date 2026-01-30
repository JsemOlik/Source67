#include "SceneHierarchyPanel.h"
#include "Core/Application.h"
#include "Core/Logger.h"
#include "Core/UndoSystem.h"
#include "Renderer/ScriptRegistry.h"
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_internal.h>

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

  if (ImGui::BeginPopupContextWindow(NULL, ImGuiPopupFlags_NoOpenOverItems)) {
    if (ImGui::BeginMenu("New Object")) {
      if (ImGui::MenuItem("Cube"))
        m_PendingCreateType = CreatePrimitiveType::Cube;
      if (ImGui::MenuItem("Sphere"))
        m_PendingCreateType = CreatePrimitiveType::Sphere;
      if (ImGui::MenuItem("Cylinder"))
        m_PendingCreateType = CreatePrimitiveType::Cylinder;

      ImGui::EndMenu();
    }
    ImGui::EndPopup();
  }

  if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
    m_SelectionContext = nullptr;

  // Defer deletion
  // Defer deletion
  if (m_EntityToDelete) {
    if (m_EntityToDelete->Name == "Player") {
      m_EntityToDelete = nullptr;
    } else {
      (*m_Context)->RemoveEntity(m_EntityToDelete);
      if (m_SelectionContext == m_EntityToDelete)
        m_SelectionContext = nullptr;
      m_EntityToDelete = nullptr;
    }
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
    if (entity->Name != "Player") {
      if (ImGui::MenuItem("Rename"))
        m_RenamingEntity = entity;

      if (ImGui::MenuItem("Delete Geometry"))
        m_EntityToDelete = entity;
    } else {
      ImGui::TextDisabled("Player Object (Protected)");
    }

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

static void DrawFloatControl(const std::string &label, float &value,
                             float resetValue = 0.0f,
                             float columnWidth = 150.0f) {
  ImGui::PushID(label.c_str());

  ImGui::Columns(2);
  ImGui::SetColumnWidth(0, columnWidth);
  ImGui::Text("%s", label.c_str());
  ImGui::NextColumn();

  float availWidth = ImGui::GetContentRegionAvail().x;
  float width =
      std::min(availWidth, 400.0f); // Max 400px to prevent massive sliders
  ImGui::PushItemWidth(width);
  ImGui::DragFloat("##value", &value, 0.1f, 0.0f, 0.0f, "%.2f");
  ImGui::PopItemWidth();

  ImGui::Columns(1);
  ImGui::PopID();
}

static void DrawVec2Control(const std::string &label, glm::vec2 &values,
                            float resetValue = 0.0f,
                            float columnWidth = 150.0f) {
  ImGui::PushID(label.c_str());

  ImGui::Columns(2);
  ImGui::SetColumnWidth(0, columnWidth);
  ImGui::Text("%s", label.c_str());
  ImGui::NextColumn();

  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

  float lineHeight =
      ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
  ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};
  float widthEach =
      (ImGui::GetContentRegionAvail().x - 2.0f * buttonSize.x) / 2.0f;

  // X Axis
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
  ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
  if (ImGui::Button("X", buttonSize)) {
    values.x = resetValue;
  }
  ImGui::PopFont();
  ImGui::PopStyleColor(3);

  ImGui::SameLine();
  ImGui::PushItemWidth(widthEach);
  if (ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f")) {
  }
  ImGui::PopItemWidth();
  ImGui::SameLine();

  // Y Axis
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
  ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
  if (ImGui::Button("Y", buttonSize)) {
    values.y = resetValue;
  }
  ImGui::PopFont();
  ImGui::PopStyleColor(3);

  ImGui::SameLine();
  ImGui::PushItemWidth(widthEach);
  if (ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f")) {
  }
  ImGui::PopItemWidth();

  ImGui::PopStyleVar();
  ImGui::Columns(1);
  ImGui::PopID();
}

static bool DrawVec3Control(const std::string &label, glm::vec3 &values,
                            float resetValue = 0.0f,
                            float columnWidth = 150.0f) {
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
  float widthEach =
      (ImGui::GetContentRegionAvail().x - 3.0f * buttonSize.x) / 3.0f;

  // X Axis
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
  ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
  if (ImGui::Button("X", buttonSize)) {
    values.x = resetValue;
    changed = true;
  }
  ImGui::PopFont();
  ImGui::PopStyleColor(3);

  ImGui::SameLine();
  ImGui::PushItemWidth(widthEach);
  if (ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f"))
    changed = true;
  ImGui::PopItemWidth();
  ImGui::SameLine();

  // Y Axis
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
  ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
  if (ImGui::Button("Y", buttonSize)) {
    values.y = resetValue;
    changed = true;
  }
  ImGui::PopFont();
  ImGui::PopStyleColor(3);

  ImGui::SameLine();
  ImGui::PushItemWidth(widthEach);
  if (ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f"))
    changed = true;
  ImGui::PopItemWidth();
  ImGui::SameLine();

  // Z Axis
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                        ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
  ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
  if (ImGui::Button("Z", buttonSize)) {
    values.z = resetValue;
    changed = true;
  }
  ImGui::PopFont();
  ImGui::PopStyleColor(3);

  ImGui::SameLine();
  ImGui::PushItemWidth(widthEach);
  if (ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f"))
    changed = true;
  ImGui::PopItemWidth();

  ImGui::PopStyleVar();

  ImGui::Columns(1);
  ImGui::PopID();

  return changed;
}

template <typename UIFunction>
static void DrawComponent(const std::string &name, UIFunction uiFunction) {
  const ImGuiTreeNodeFlags treeNodeFlags =
      ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
      ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowOverlap |
      ImGuiTreeNodeFlags_FramePadding;

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
  float lineHeight =
      ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
  ImGui::Separator();
  bool open = ImGui::TreeNodeEx((void *)name.c_str(), treeNodeFlags, "%s",
                                name.c_str());
  ImGui::PopStyleVar();
  if (open) {
    uiFunction();
    ImGui::TreePop();
  }
}

void SceneHierarchyPanel::DrawProperties(Ref<Entity> entity) {
  if (m_SelectionIsMaterial) {
    DrawComponent("Material Properties", [&]() {
      if (entity->Material.AlbedoMap) {
        ImGui::Text("Texture: %s",
                    std::filesystem::path(entity->Material.AlbedoMap->GetPath())
                        .filename()
                        .string()
                        .c_str());
        DrawVec2Control("Tiling", entity->Material.Tiling, 1.0f);
        ImGui::Spacing();
      }
    });
  } else {
    DrawComponent("Transform", [&]() {
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
    });

    if (entity->Name == "Player") {
      DrawComponent("Player Camera", [&]() {
        DrawFloatControl("FOV", entity->CameraFOV, 45.0f);
      });

      DrawComponent("Movement Settings", [&]() {
        ImGui::Spacing();
        ImGui::TextDisabled("Speeds");
        DrawFloatControl("Max Run Speed", entity->Movement.MaxSpeed);
        DrawFloatControl("Max Sprint Speed", entity->Movement.MaxSprintSpeed);
        DrawFloatControl("Max Crouch Speed", entity->Movement.MaxCrouchSpeed);
        DrawFloatControl("Stop Speed", entity->Movement.StopSpeed);

        ImGui::Spacing();
        ImGui::TextDisabled("Physics");
        DrawFloatControl("Acceleration", entity->Movement.Acceleration);
        DrawFloatControl("Air Acceleration", entity->Movement.AirAcceleration);
        DrawFloatControl("Friction", entity->Movement.Friction);
        DrawFloatControl("Max Air Wish Speed",
                         entity->Movement.MaxAirWishSpeed);

        ImGui::Spacing();
        ImGui::TextDisabled("Gravity & Jump");
        DrawFloatControl("Jump Velocity", entity->Movement.JumpVelocity);
        DrawFloatControl("Gravity", entity->Movement.Gravity);
      });
    }

    DrawComponent("Mesh", [&]() {
      // Simple mesh selection placeholder text for now, could expand later
      ImGui::Text("Mesh Asset: %s", entity->MeshPath.c_str());
      ImGui::Checkbox("Collidable", &entity->Collidable);
      
      if (ImGui::Checkbox("Anchored", &entity->Anchored)) {
        // Recreate physics body when Anchored changes
        Application::Get().OnEntityCollidableChanged(entity);
      }
    });

        DrawVec2Control("Tiling", entity->Material.Tiling, 1.0f);
      });
    }

    DrawComponent("Tags", [&]() {
      static char tagBuffer[256] = "";
      ImGui::InputText("##NewTag", tagBuffer, sizeof(tagBuffer));
      ImGui::SameLine();
      if (ImGui::Button("Add Tag")) {
        if (entity->Tags.size() < 10 && strlen(tagBuffer) > 0) {
          entity->Tags.push_back(tagBuffer);
          tagBuffer[0] = '\0';
          m_SceneModified = true;
        }
      }

      ImGui::Spacing();
      for (int i = 0; i < entity->Tags.size(); i++) {
        ImGui::PushID(i);
        ImGui::Text("%s", entity->Tags[i].c_str());
        ImGui::SameLine();
        if (ImGui::Button("X")) {
          entity->Tags.erase(entity->Tags.begin() + i);
          m_SceneModified = true;
          ImGui::PopID();
          break;
        }
        ImGui::PopID();
      }
    });

    DrawComponent("Scripts", [&]() {
      if (ImGui::Button("Add Script")) {
        ImGui::OpenPopup("AddScriptPopup");
      }

      if (ImGui::BeginPopup("AddScriptPopup")) {
        for (auto const &[name, func] :
             ScriptRegistry::Get().GetAvailableScripts()) {
          if (ImGui::MenuItem(name.c_str())) {
            NativeScriptComponent nsc;
            ScriptRegistry::Get().Bind(name, nsc);
            entity->Scripts.push_back(nsc);
            m_SceneModified = true;
          }
        }
        ImGui::EndPopup();
      }

      ImGui::Spacing();
      for (int i = 0; i < entity->Scripts.size(); i++) {
        ImGui::PushID(i);
        ImGui::Text("%s", entity->Scripts[i].Name.c_str());
        ImGui::SameLine();
        if (ImGui::Button("Remove")) {
          if (entity->Scripts[i].DestroyScript)
            entity->Scripts[i].DestroyScript(&entity->Scripts[i]);
          entity->Scripts.erase(entity->Scripts.begin() + i);
          m_SceneModified = true;
          ImGui::PopID();
          break;
        }
        ImGui::PopID();
      }
    });
  }
}

} // namespace S67
