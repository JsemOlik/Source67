#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include "Core/Base.h"

namespace S67 {

    class PhysicsShapes {
    public:
        static JPH::Ref<JPH::Shape> CreateBox(const glm::vec3& halfExtent) {
            return new JPH::BoxShape(JPH::Vec3(halfExtent.x, halfExtent.y, halfExtent.z));
        }

        static JPH::Ref<JPH::Shape> CreateSphere(float radius) {
            return new JPH::SphereShape(radius);
        }
    };

}
