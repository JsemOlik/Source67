#pragma once

#include "Core/Base.h"

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>


#include "Core/Timestep.h"
#include <glm/glm.hpp>

namespace S67 {

namespace Layers {
static constexpr JPH::ObjectLayer NON_MOVING = 0;
static constexpr JPH::ObjectLayer MOVING = 1;
static constexpr uint32_t NUM_LAYERS = 2;
} // namespace Layers

class PhysicsSystem {
public:
  static void Init();
  static void Shutdown();

  static void OnUpdate(Timestep ts);

  static JPH::PhysicsSystem &GetPhysicsSystem() { return *s_PhysicsSystem; }
  static JPH::BodyInterface &GetBodyInterface() {
    return s_PhysicsSystem->GetBodyInterface();
  }

  static JPH::BodyID Raycast(const glm::vec3 &origin,
                             const glm::vec3 &direction, float distance);

  static const JPH::BroadPhaseLayerFilter &GetBroadPhaseLayerFilter();

  static const JPH::ObjectLayerFilter &GetObjectLayerFilter();

  static void SetGravity(float gravity);

private:
  static JPH::PhysicsSystem *s_PhysicsSystem;
  static JPH::TempAllocatorImpl *s_TempAllocator;
  static JPH::JobSystemThreadPool *s_JobSystem;
};

} // namespace S67
