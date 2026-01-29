#pragma once

#include "Renderer/Entity.h"

namespace S67 {

class ScriptableEntity {
public:
  virtual ~ScriptableEntity() {}

  template <typename T> T &GetComponent() { return m_Entity.GetComponent<T>(); }

  // Shortcut to Entity Transform
  // Transform& GetTransform() { return
  // GetComponent<TransformComponent>().Transform; }

protected:
  virtual void OnCreate() {}
  virtual void OnUpdate(float ts) {}
  virtual void OnDestroy() {}

private:
  Entity m_Entity;
  friend class Scene; // Allowed to set m_Entity
};

} // namespace S67
