#include "Renderer/ScriptableEntity.h"
#include "Physics/PhysicsSystem.h"
#include "Renderer/HUDRenderer.h"
#include "Core/Application.h"
#include <glm/gtc/matrix_transform.hpp>

namespace S67 {

Entity* ScriptableEntity::Raycast(float distance) {
    auto& transform = GetTransform();
    
    // Calculate ray from camera if we are a player-like entity, 
    // or just use entity forward for now as a simple abstraction.
    // In this engine, the player is usually the one raycasting.
    
    glm::vec3 origin = transform.Position + glm::vec3(0.0f, 1.7f, 0.0f);
    
    float pitch = glm::radians(transform.Rotation.x);
    float yaw = glm::radians(transform.Rotation.y - 90.0f);
    
    glm::vec3 direction;
    direction.x = cos(pitch) * cos(yaw);
    direction.y = sin(pitch);
    direction.z = cos(pitch) * sin(yaw);
    direction = glm::normalize(direction);

    JPH::BodyID hitBody = PhysicsSystem::Raycast(origin, direction, distance);

    if (!hitBody.IsInvalid()) {
        auto& bodyInterface = PhysicsSystem::GetBodyInterface();
        return (Entity*)bodyInterface.GetUserData(hitBody);
    }
    
    return nullptr;
}

void ScriptableEntity::SetText(const std::string& id, const std::string& text, const glm::vec2& pos, float scale, const glm::vec4& color) {
    HUDRenderer::SetText(id, text, pos, scale, color);
}

void ScriptableEntity::ClearText(const std::string& id) {
    HUDRenderer::ClearText(id);
}

void ScriptableEntity::PrintHUD(const std::string& text, const glm::vec4& color) {
    HUDRenderer::QueueString(text, color);
}

bool ScriptableEntity::HasTag(const std::string& tag) {
    return m_Entity->HasTag(tag);
}

Entity* ScriptableEntity::FindEntity(const std::string& name) {
    return Application::Get().GetScene().FindEntityByName(name).get();
}

void ScriptableEntity::Move(const glm::vec3& delta) {
    m_Entity->Transform.Position += delta;
}

void ScriptableEntity::Move(Entity* other, const glm::vec3& delta) {
    if (other) other->Transform.Position += delta;
}

void ScriptableEntity::SetPosition(const glm::vec3& pos) {
    m_Entity->Transform.Position = pos;
}

void ScriptableEntity::SetPosition(Entity* other, const glm::vec3& pos) {
    if (other) other->Transform.Position = pos;
}

void ScriptableEntity::Rotate(const glm::vec3& eulerDelta) {
    m_Entity->Transform.Rotation += eulerDelta;
}

void ScriptableEntity::Rotate(Entity* other, const glm::vec3& eulerDelta) {
    if (other) other->Transform.Rotation += eulerDelta;
}

bool ScriptableEntity::IsKeyPressed(int key) {
    return Input::IsKeyPressed(key);
}

} // namespace S67
