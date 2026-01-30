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

protected:
  virtual void OnCreate() {}
  virtual void OnUpdate(float ts) {}
  virtual void OnEvent(Event &e) {}
  virtual void OnDestroy() {}

private:
  Entity *m_Entity = nullptr;
  friend class Scene; // Allowed to set m_Entity
};

} // namespace S67
