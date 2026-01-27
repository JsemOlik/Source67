#pragma once

#include "Core/Base.h"
#include "Renderer/Scene.h"
#include <imgui.h>

namespace S67 {

class SceneHierarchyPanel {
public:
  enum class CreatePrimitiveType { None = 0, Cube, Sphere, Cylinder };

  SceneHierarchyPanel() = default;
  SceneHierarchyPanel(const Scope<Scene> &context);

  void SetContext(const Scope<Scene> &context);

  void OnImGuiRender();

  Ref<Entity> GetSelectedEntity() const { return m_SelectionContext; }
  void SetSelectedEntity(Ref<Entity> entity) { m_SelectionContext = entity; }

  CreatePrimitiveType GetPendingCreateType() const {
    return m_PendingCreateType;
  }
  void ClearPendingCreateType() {
    m_PendingCreateType = CreatePrimitiveType::None;
  }

private:
  void DrawEntityNode(Ref<Entity> entity);
  void DrawProperties(Ref<Entity> entity);

private:
  const Scope<Scene> *m_Context = nullptr;
  Ref<Entity> m_SelectionContext;
  bool m_SelectionIsMaterial = false;
  Ref<Entity> m_EntityToDelete; // Defer deletion
  Ref<Entity> m_RenamingEntity; // Rename state
  CreatePrimitiveType m_PendingCreateType = CreatePrimitiveType::None;
};

} // namespace S67
