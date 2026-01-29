#pragma once

#include "Renderer/Mesh.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>

#include <glm/glm.hpp>
#include <vector>


namespace S67 {

class PhysicsShapes {
public:
  static JPH::Ref<JPH::Shape> CreateBox(const glm::vec3 &halfExtent) {
    return new JPH::BoxShape(
        JPH::Vec3(halfExtent.x, halfExtent.y, halfExtent.z));
  }

  static JPH::Ref<JPH::Shape> CreateSphere(float radius) {
    return new JPH::SphereShape(radius);
  }

  static JPH::Ref<JPH::Shape> CreateMeshShape(const MeshGeometry &geometry) {
    JPH::TriangleList triangles;
    triangles.reserve(geometry.Indices.size() / 3);
    for (size_t i = 0; i < geometry.Indices.size(); i += 3) {
      const auto &v0 = geometry.Vertices[geometry.Indices[i]];
      const auto &v1 = geometry.Vertices[geometry.Indices[i + 1]];
      const auto &v2 = geometry.Vertices[geometry.Indices[i + 2]];
      triangles.push_back(JPH::Triangle(JPH::Float3(v0.x, v0.y, v0.z),
                                        JPH::Float3(v1.x, v1.y, v1.z),
                                        JPH::Float3(v2.x, v2.y, v2.z)));
    }
    JPH::MeshShapeSettings settings(triangles);
    JPH::Shape::ShapeResult result = settings.Create();
    return result.IsValid() ? result.Get() : nullptr;
  }

  static JPH::Ref<JPH::Shape>
  CreateConvexHullShape(const MeshGeometry &geometry) {
    std::vector<JPH::Vec3> points;
    points.reserve(geometry.Vertices.size());
    for (const auto &v : geometry.Vertices) {
      points.push_back(JPH::Vec3(v.x, v.y, v.z));
    }
    JPH::ConvexHullShapeSettings settings(points.data(), (int)points.size());
    JPH::Shape::ShapeResult result = settings.Create();
    return result.IsValid() ? result.Get() : nullptr;
  }
};

} // namespace S67
