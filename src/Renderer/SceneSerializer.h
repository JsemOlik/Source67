#pragma once

#include "Scene.h"

namespace S67 {

class SceneSerializer {
public:
  SceneSerializer(Scene *scene, const std::string &projectRoot = "");

  void Serialize(const std::string &filepath);
  bool Deserialize(const std::string &filepath);

private:
  std::string MakeRelative(const std::string &path);

  Scene *m_Scene;
  std::string m_ProjectRoot;
};

} // namespace S67
