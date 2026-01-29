#pragma once

#include "Core/Base.h"
#include "VertexArray.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace S67 {

struct MeshGeometry {
  std::vector<glm::vec3> Vertices;
  std::vector<uint32_t> Indices;
};

class MeshLoader {
public:
  static Ref<VertexArray> LoadOBJ(const std::string &path);
  static Ref<VertexArray> LoadSTL(const std::string &path);
  static Ref<VertexArray> LoadModel(const std::string &path);
  static MeshGeometry LoadGeometry(const std::string &path);
  static Ref<VertexArray> CreateCapsule(float radius, float height);
  static Ref<VertexArray> CreateCube();
};

} // namespace S67
