#pragma once

#include "Entity.h"
#include "Events/Event.h"
#include <string>
#include <glm/glm.hpp>

namespace S67 {

class ScriptableEntity {
public:
  virtual ~ScriptableEntity() {}

  Entity &GetEntity() { return *m_Entity; }

  // Shortcuts
  Transform &GetTransform() { return m_Entity->Transform; }

  // "Stupid Simple" API
  Entity *Raycast(float distance = 10.0f);
  void SetText(const std::string &id, const std::string &text,
               const glm::vec2 &pos = {0.5f, 0.1f}, float scale = 3.0f,
               const glm::vec4 &color = {1.0f, 1.0f, 1.0f, 1.0f});
  void ClearText(const std::string &id);
  void PrintHUD(const std::string &text,
                const glm::vec4 &color = {1.0f, 1.0f, 1.0f, 1.0f});
  bool HasTag(const std::string &tag);

  // Discovery & Manipulation
  Entity *FindEntity(const std::string &name);
  void Move(const glm::vec3 &delta);
  void Move(Entity *other, const glm::vec3 &delta);
  void SetPosition(const glm::vec3 &pos);
  void SetPosition(Entity *other, const glm::vec3 &pos);
  void Rotate(const glm::vec3 &eulerDelta);
  void Rotate(Entity *other, const glm::vec3 &eulerDelta);

  // Input
  bool IsKeyPressed(int key);

protected:
  virtual void OnCreate() {}
  virtual void OnUpdate(float ts) { (void)ts; }
  virtual void OnEvent(Event &e) { (void)e; }
  virtual void OnDestroy() {}

private:
  Entity *m_Entity = nullptr;
  friend class Scene; // Allowed to set m_Entity
};

} // namespace S67
