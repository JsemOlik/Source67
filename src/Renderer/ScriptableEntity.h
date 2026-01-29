#pragma once

#include "Entity.h"

namespace S67 {

class ScriptableEntity {
public:
  virtual ~ScriptableEntity() {}

  Entity &GetEntity() { return *m_Entity; }

  // Shortcuts
  Transform &GetTransform() { return m_Entity->Transform; }

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
