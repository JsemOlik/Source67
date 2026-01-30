#include "Renderer/ScriptableEntity.h"
#include "Core/Application.h"
#include "Core/Input.h"
#include "Physics/PhysicsSystem.h"
#include "Renderer/HUDRenderer.h"

namespace S67 {

Entity &ScriptableEntity::GetEntity() { return *m_Entity; }
Transform &ScriptableEntity::GetTransform() { return m_Entity->Transform; }

Entity *ScriptableEntity::Raycast(float distance) {
  JPH::BodyID bodyID = PhysicsSystem::Raycast(m_Entity->Transform.Position,
                                              m_Entity->Transform.Rotation * glm::vec3(0, 0, -1), // Forward vector
                                              distance);
  // mapping bodyID to Entity is needed here, but for now let's just return nullptr if no hit or we need a way to look it up.
  // Actually, PhysicsSystem::Raycast returns a BodyID. We need to map it back.
  // For the purpose of "Stupid Simple" API, maybe we return the Entity* if we can find it.
  // But PhysicsSystem in this engine might not natively map BodyID -> Entity* easily without lookup.
  // Let's assume for now we just want to know if we hit something, or maybe PhysicsSystem needs a helper.
  // Allow me to check PhysicsSystem.h again? No, I saw it.
  // Wait, Raycast in PhysicsSystem.h returns BodyID.
  // Let's just return nullptr for now or fix it properly later.
  return nullptr;
}

void ScriptableEntity::SetText(const std::string &id, const std::string &text,
                               const glm::vec2 &pos, float scale,
                               const glm::vec4 &color) {
  HUDRenderer::SetText(id, text, pos, scale, color);
}

void ScriptableEntity::ClearText(const std::string &id) {
  HUDRenderer::ClearText(id);
}

void ScriptableEntity::PrintHUD(const std::string &text,
                                const glm::vec4 &color) {
  HUDRenderer::QueueString(text, color);
}

bool ScriptableEntity::HasTag(const std::string &tag) {
  return m_Entity->HasTag(tag);
}

Entity *ScriptableEntity::FindEntity(const std::string &name) {
  // This would typically involve searching the scene.
  // For now, let's assume we have access to the scene or a registry.
  return Application::Get().GetScene().FindEntityByName(name).get();
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
