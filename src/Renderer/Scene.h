#pragma once

#include "Camera.h"
#include "Core/Base.h"
#include "Entity.h"
#include <algorithm>
#include <vector>

namespace S67 {

class Scene {
public:
  Scene() = default;

  void AddEntity(const Ref<Entity> &entity) { m_Entities.push_back(entity); }
  void RemoveEntity(const Ref<Entity> &entity) {
    auto it = std::find(m_Entities.begin(), m_Entities.end(), entity);
    if (it != m_Entities.end())
      m_Entities.erase(it);
  }
  void EnsurePlayerExists();
  void OnUpdate(float ts);
  void InstantiateScripts();
  Ref<Entity> FindEntityByName(const std::string &name);
  void Clear() { m_Entities.clear(); }
  const std::vector<Ref<Entity>> &GetEntities() const { return m_Entities; }

  const std::string &GetUIPath() const { return m_UIPath; }
  void SetUIPath(const std::string &path) { m_UIPath = path; }

private:
  std::vector<Ref<Entity>> m_Entities;
  std::string m_UIPath;
};

} // namespace S67
