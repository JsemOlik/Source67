#include "Renderer/ScriptableEntity.h"
#include "Core/Application.h"
#include "Core/Input.h"
#include "Physics/PhysicsSystem.h"

#include "Renderer/ScriptableEntity.h"

namespace S67 {

Entity &ScriptableEntity::GetEntity() { return *m_Entity; }
Transform &ScriptableEntity::GetTransform() { return m_Entity->Transform; }

Entity *ScriptableEntity::Raycast(float distance) {
  return PhysicsSystem::Get().Raycast(m_Entity->Transform.Position,
                                      m_Entity->Transform.Rotation, distance);
}

void ScriptableEntity::SetText(const std::string &id, const std::string &text,
                               const glm::vec2 &pos, float scale,
                               const glm::vec4 &color) {
  Application::Get().GetHUD().SetText(id, text, pos, scale, color);
}

void ScriptableEntity::ClearText(const std::string &id) {
  Application::Get().GetHUD().ClearText(id);
}

void ScriptableEntity::PrintHUD(const std::string &text,
                                const glm::vec4 &color) {
  Application::Get().GetHUD().PrintHUD(text, color);
}

bool ScriptableEntity::HasTag(const std::string &tag) {
  return m_Entity->HasTag(tag);
}

Entity *ScriptableEntity::FindEntity(const std::string &name) {
  // This would typically involve searching the scene.
  // For now, let's assume we have access to the scene or a registry.
  return Application::Get().GetActiveScene()->FindEntityByName(name);
}

void ScriptableEntity::Move(const glm::vec3 &delta) {
  m_Entity->Transform.Position += delta;
}

void ScriptableEntity::Move(Entity *other, const glm::vec3 &delta) {
  if (other)
    other->Transform.Position += delta;
}

void ScriptableEntity::SetPosition(const glm::vec3 &pos) {
  m_Entity->Transform.Position = pos;
}

void ScriptableEntity::SetPosition(Entity *other, const glm::vec3 &pos) {
  if (other)
    other->Transform.Position = pos;
}

void ScriptableEntity::Rotate(const glm::vec3 &eulerDelta) {
  m_Entity->Transform.Rotation += eulerDelta;
}

void ScriptableEntity::Rotate(Entity *other, const glm::vec3 &eulerDelta) {
  if (other)
    other->Transform.Rotation += eulerDelta;
}

bool ScriptableEntity::IsKeyPressed(int key) {
  return Input::IsKeyPressed(key);
}

} // namespace S67
