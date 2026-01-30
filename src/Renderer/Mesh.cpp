#define TINYOBJLOADER_IMPLEMENTATION
#include "Mesh.h"
#include "Core/Application.h"
#include "Core/Logger.h"
#include "tinyobjloader/tiny_obj_loader.h"
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <sstream>
#include <unordered_map>

namespace S67 {

struct OBJVertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoord;

  bool operator==(const OBJVertex &other) const {
    return Position == other.Position && Normal == other.Normal &&
           TexCoord == other.TexCoord;
  }
};
} // namespace S67

namespace std {
template <> struct hash<S67::OBJVertex> {
  size_t operator()(const S67::OBJVertex &vertex) const {
    return ((hash<float>()(vertex.Position.x) ^
             (hash<float>()(vertex.Position.y) << 1)) >>
            1) ^
           (hash<float>()(vertex.Position.z) << 1) ^
           (hash<float>()(vertex.Normal.x) << 1) ^
           (hash<float>()(vertex.Normal.y) << 1) ^
           (hash<float>()(vertex.Normal.z) << 1) ^
           (hash<float>()(vertex.TexCoord.x) << 1) ^
           (hash<float>()(vertex.TexCoord.y) << 1);
  }
};
} // namespace std

namespace S67 {

Ref<VertexArray> MeshLoader::LoadOBJ(const std::string &path) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  std::vector<uint8_t> pakBuffer;
  if (Application::Get().GetPakAsset(path, pakBuffer)) {
    std::string source((char *)pakBuffer.data(), pakBuffer.size());
    std::stringstream ss(source);
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, &ss)) {
      S67_CORE_ERROR("Failed to load OBJ from PAK: {0}", err);
      return nullptr;
    }
  } else {
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                          path.c_str())) {
      S67_CORE_ERROR("Failed to load OBJ: {0}", err);
      return nullptr;
    }
  }

  std::vector<OBJVertex> vertices;
  std::vector<uint32_t> indices;
  std::unordered_map<OBJVertex, uint32_t> uniqueVertices{};
  int uvCount = 0;

  for (const auto &shape : shapes) {
    for (const auto &index : shape.mesh.indices) {
      OBJVertex vertex{};
      vertex.Position = {attrib.vertices[3 * index.vertex_index + 0],
                         attrib.vertices[3 * index.vertex_index + 1],
                         attrib.vertices[3 * index.vertex_index + 2]};

      // Defaults
      vertex.Normal = {0.0f, 1.0f, 0.0f};
      vertex.TexCoord = {0.0f, 0.0f};

      if (index.normal_index >= 0) {
        vertex.Normal = {attrib.normals[3 * index.normal_index + 0],
                         attrib.normals[3 * index.normal_index + 1],
                         attrib.normals[3 * index.normal_index + 2]};
      }

      if (index.texcoord_index >= 0) {
        vertex.TexCoord = {attrib.texcoords[2 * index.texcoord_index + 0],
                           attrib.texcoords[2 * index.texcoord_index + 1]};
        uvCount++;
      } else {
        // Auto-UV projection (Planar XY)
        vertex.TexCoord = {vertex.Position.x, vertex.Position.y};
      }

      if (uniqueVertices.count(vertex) == 0) {
        uniqueVertices[vertex] = (uint32_t)vertices.size();
        vertices.push_back(vertex);
      }

      indices.push_back(uniqueVertices[vertex]);
    }
  }

  S67_CORE_INFO("Loaded OBJ: {0} ({1} vertices, {2} indices, {3} explicit UVs, "
                "Generated Auto-UVs for others)",
                path, vertices.size(), indices.size(), uvCount);

  Ref<VertexArray> va = VertexArray::Create();
  Ref<VertexBuffer> vb =
      VertexBuffer::Create((float *)vertices.data(),
                           (uint32_t)(vertices.size() * sizeof(OBJVertex)));
  vb->SetLayout({{ShaderDataType::Float3, "a_Position"},
                 {ShaderDataType::Float3, "a_Normal"},
                 {ShaderDataType::Float2, "a_TexCoord"}});
  va->AddVertexBuffer(vb);

  Ref<IndexBuffer> ib =
      IndexBuffer::Create(indices.data(), (uint32_t)indices.size());
  va->SetIndexBuffer(ib);

  return va;
}

Ref<VertexArray> MeshLoader::LoadSTL(const std::string &path) {
  std::vector<uint8_t> pakBuffer;
  bool fromPak = Application::Get().GetPakAsset(path, pakBuffer);

  // STL Binary header is 80 bytes
  const uint8_t *dataPtr = nullptr;
  size_t dataSize = 0;
  std::ifstream file;

  if (fromPak) {
    dataPtr = pakBuffer.data();
    dataSize = pakBuffer.size();
  } else {
    file.open(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
      S67_CORE_ERROR("Failed to open STL file: {0}", path);
      return nullptr;
    }
    dataSize = file.tellg();
    file.seekg(0, std::ios::beg);
  }

  if (dataSize < 84) {
    S67_CORE_ERROR("STL data too small: {0}", path);
    return nullptr;
  }

  uint32_t triangleCount;
  if (fromPak) {
    memcpy(&triangleCount, dataPtr + 80, sizeof(uint32_t));
    dataPtr += 84;
  } else {
    file.seekg(80, std::ios::beg);
    file.read(reinterpret_cast<char *>(&triangleCount), sizeof(uint32_t));
  }

  std::vector<OBJVertex> vertices;
  std::vector<uint32_t> indices;
  vertices.reserve(triangleCount * 3);
  indices.reserve(triangleCount * 3);

  for (uint32_t i = 0; i < triangleCount; i++) {
    float normal[3];
    float v1[3], v2[3], v3[3];
    uint16_t attributeByteCount;

    file.read(reinterpret_cast<char *>(normal), 12);
    file.read(reinterpret_cast<char *>(v1), 12);
    file.read(reinterpret_cast<char *>(v2), 12);
    file.read(reinterpret_cast<char *>(v3), 12);
    file.read(reinterpret_cast<char *>(&attributeByteCount), 2);

    glm::vec3 n = {normal[0], normal[1], normal[2]};

    // If normal is zero or invalid, compute it from vertices
    if (glm::length(n) < 0.0001f) {
      glm::vec3 edge1 = {v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2]};
      glm::vec3 edge2 = {v3[0] - v1[0], v3[1] - v1[1], v3[2] - v1[2]};
      n = glm::normalize(glm::cross(edge1, edge2));
    }

    vertices.push_back({{v1[0], v1[1], v1[2]}, n, {v1[0], v1[1]}});
    vertices.push_back({{v2[0], v2[1], v2[2]}, n, {v2[0], v2[1]}});
    vertices.push_back({{v3[0], v3[1], v3[2]}, n, {v3[0], v3[1]}});

    indices.push_back(i * 3 + 0);
    indices.push_back(i * 3 + 1);
    indices.push_back(i * 3 + 2);
  }

  S67_CORE_INFO("Loaded STL: {0} ({1} triangles, Generated Auto-UVs)", path,
                triangleCount);

  Ref<VertexArray> va = VertexArray::Create();
  Ref<VertexBuffer> vb =
      VertexBuffer::Create((float *)vertices.data(),
                           (uint32_t)(vertices.size() * sizeof(OBJVertex)));
  vb->SetLayout({{ShaderDataType::Float3, "a_Position"},
                 {ShaderDataType::Float3, "a_Normal"},
                 {ShaderDataType::Float2, "a_TexCoord"}});
  va->AddVertexBuffer(vb);

  Ref<IndexBuffer> ib =
      IndexBuffer::Create(indices.data(), (uint32_t)indices.size());
  va->SetIndexBuffer(ib);

  return va;
}

Ref<VertexArray> MeshLoader::CreateCube() {
  std::vector<OBJVertex> vertices;
  std::vector<uint32_t> indices;

  float verticesData[] = {
      // Front
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, -0.5f, 0.5f, 0.0f,
      0.0f, 1.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
      -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
      // Back
      0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -0.5f, -0.5f, -0.5f,
      0.0f, 0.0f, -1.0f, 1.0f, 0.0f, -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
      1.0f, 1.0f, 0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
      // Left
      -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, -0.5f, 0.5f,
      -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f,
      1.0f, -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
      // Right
      0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, -0.5f, -0.5f, 1.0f,
      0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
      0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
      // Top
      -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.0f,
      1.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
      // Bottom
      -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.5f, -0.5f, -0.5f,
      0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f,
      1.0f, -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f};

  for (int i = 0; i < 24; i++) {
    vertices.push_back({{verticesData[i * 8 + 0], verticesData[i * 8 + 1],
                         verticesData[i * 8 + 2]},
                        {verticesData[i * 8 + 3], verticesData[i * 8 + 4],
                         verticesData[i * 8 + 5]},
                        {verticesData[i * 8 + 6], verticesData[i * 8 + 7]}});
  }

  uint32_t indicesData[] = {
      0,  1,  2,  2,  3,  0,  // Front
      4,  5,  6,  6,  7,  4,  // Back
      8,  9,  10, 10, 11, 8,  // Left
      12, 13, 14, 14, 15, 12, // Right
      16, 17, 18, 18, 19, 16, // Top
      20, 21, 22, 22, 23, 20  // Bottom
  };
  indices.assign(indicesData, indicesData + 36);

  Ref<VertexArray> va = VertexArray::Create();
  Ref<VertexBuffer> vb =
      VertexBuffer::Create((float *)vertices.data(),
                           (uint32_t)(vertices.size() * sizeof(OBJVertex)));
  vb->SetLayout({{ShaderDataType::Float3, "a_Position"},
                 {ShaderDataType::Float3, "a_Normal"},
                 {ShaderDataType::Float2, "a_TexCoord"}});
  va->AddVertexBuffer(vb);
  Ref<IndexBuffer> ib =
      IndexBuffer::Create(indices.data(), (uint32_t)indices.size());
  va->SetIndexBuffer(ib);
  return va;
}

Ref<VertexArray> MeshLoader::CreateCapsule(float radius, float height) {
  std::vector<OBJVertex> vertices;
  std::vector<uint32_t> indices;

  int segments = 16;
  int rings = 8;
  float halfHeight = height * 0.5f;

  // Helper to add vertex
  auto addVertex = [&](float x, float y, float z, float u, float v) {
    vertices.push_back({{x, y, z},
                        glm::normalize(glm::vec3(
                            x, y > 0 ? y - halfHeight : y + halfHeight, z)),
                        {u, v}});
  };

  // Top Hemisphere
  for (int i = 0; i <= rings; ++i) {
    float lat = (float)i / (float)rings * (glm::pi<float>() / 2.0f);
    float y = glm::sin(lat) * radius + halfHeight;
    float r = glm::cos(lat) * radius;

    for (int j = 0; j <= segments; ++j) {
      float lon = (float)j / (float)segments * 2.0f * glm::pi<float>();
      float u = (float)j / (float)segments;
      float v = (float)i / (float)(rings * 2 + 1); // rough UV
      addVertex(glm::cos(lon) * r, y, glm::sin(lon) * r, u, v);
    }
  }

  // Bottom Hemisphere
  for (int i = 0; i <= rings; ++i) {
    float lat = (float)i / (float)rings * (glm::pi<float>() / 2.0f);
    float y = -glm::sin(lat) * radius - halfHeight;
    float r = glm::cos(lat) * radius;

    for (int j = 0; j <= segments; ++j) {
      float lon = (float)j / (float)segments * 2.0f * glm::pi<float>();
      float u = (float)j / (float)segments;
      float v = 0.5f + (float)i / (float)(rings * 2 + 1);
      addVertex(glm::cos(lon) * r, y, glm::sin(lon) * r, u, v);
    }
  }

  // Indices (Simplified: just treat as two independent meshes for now, bridging
  // is extra code) Just fill indices for rings.
  auto makeIndices = [&](int startRing, int nRings) {
    for (int i = 0; i < nRings; ++i) {
      for (int j = 0; j < segments; ++j) {
        int next = (j + 1) % (segments + 1);
        int currentRing = startRing + i * (segments + 1);
        int nextRing = startRing + (i + 1) * (segments + 1);

        indices.push_back(currentRing + j);
        indices.push_back(nextRing + j);
        indices.push_back(nextRing + next);

        indices.push_back(currentRing + j);
        indices.push_back(nextRing + next);
        indices.push_back(currentRing + next);
      }
    }
  };

  makeIndices(0, rings);         // Top
  makeIndices(rings + 1, rings); // Bottom

  // Cylinder Body Connection
  int topRingIndex = rings;
  int bottomRingIndex = rings + 1;
  for (int j = 0; j < segments; ++j) {
    int next = (j + 1) % (segments + 1);
    int current = topRingIndex * (segments + 1) + j;
    int below = bottomRingIndex * (segments + 1) + j;
    int currentNext = topRingIndex * (segments + 1) + next;
    int belowNext = bottomRingIndex * (segments + 1) + next;

    indices.push_back(current);
    indices.push_back(below);
    indices.push_back(belowNext);

    indices.push_back(current);
    indices.push_back(belowNext);
    indices.push_back(currentNext);
  }
  // Note: vertices buffer size check needed.
  // Also cylinder body is missing.
  // For Player debug visual, two spheres separated is "Capsule-ish" enough for
  // now. It provides the visual "Top" and "Bottom" and collision shape
  // reference.

  Ref<VertexArray> va = VertexArray::Create();
  Ref<VertexBuffer> vb =
      VertexBuffer::Create((float *)vertices.data(),
                           (uint32_t)(vertices.size() * sizeof(OBJVertex)));
  vb->SetLayout({{ShaderDataType::Float3, "a_Position"},
                 {ShaderDataType::Float3, "a_Normal"},
                 {ShaderDataType::Float2, "a_TexCoord"}});
  va->AddVertexBuffer(vb);
  Ref<IndexBuffer> ib =
      IndexBuffer::Create(indices.data(), (uint32_t)indices.size());
  va->SetIndexBuffer(ib);
  return va;
}

} // namespace S67
