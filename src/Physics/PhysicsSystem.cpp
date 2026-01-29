#include "PhysicsSystem.h"
#include "Core/Assert.h"
#include "Core/Console/ConVar.h"
#include "Core/Logger.h"
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include <Jolt/Physics/Collision/RayCast.h>


namespace S67 {

// Fixed timestep for physics (66Hz to match Source Engine tick rate)
namespace {
static float s_PhysicsAccumulator = 0.0f;
static constexpr float FIXED_PHYSICS_DT =
    1.0f / 66.0f; // ~0.015151515f (15.15ms)
static constexpr int MAX_PHYSICS_STEPS = 5;

static void OnGravityChanged(ConVar *var, const std::string &oldValue,
                             float flOldValue);
static ConVar sv_gravity("sv_gravity", "800.0",
                         FCVAR_ARCHIVE | FCVAR_REPLICATED | FCVAR_NOTIFY,
                         "World gravity", OnGravityChanged);
} // namespace

// Callback implementation
static void OnGravityChanged(ConVar *var, const std::string &oldValue,
                             float flOldValue) {
  PhysicsSystem::SetGravity(var->GetFloat());
}

// --- Jolt Boilerplate ---

namespace BroadPhaseLayers {
static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
static constexpr JPH::BroadPhaseLayer MOVING(1);
static constexpr uint32_t NUM_LAYERS = 2;
} // namespace BroadPhaseLayers

class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
public:
  BPLayerInterfaceImpl() {
    mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
    mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
  }

  virtual uint32_t GetNumBroadPhaseLayers() const override {
    return BroadPhaseLayers::NUM_LAYERS;
  }
  virtual JPH::BroadPhaseLayer
  GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override {
    S67_CORE_ASSERT(inLayer < Layers::NUM_LAYERS, "Invalid layer");
    return mObjectToBroadPhase[inLayer];
  }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
  virtual const char *
  GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override {
    switch ((JPH::BroadPhaseLayer::Type)inLayer) {
    case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
      return "NON_MOVING";
    case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
      return "MOVING";
    default:
      S67_CORE_ASSERT(false, "Invalid layer");
      return "INVALID";
    }
  }
#endif
private:
  JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
};

class ObjectVsBroadPhaseLayerFilterImpl
    : public JPH::ObjectVsBroadPhaseLayerFilter {
public:
  virtual bool ShouldCollide(JPH::ObjectLayer inLayer1,
                             JPH::BroadPhaseLayer inLayer2) const override {
    switch (inLayer1) {
    case Layers::NON_MOVING:
      return inLayer2 == BroadPhaseLayers::MOVING;
    case Layers::MOVING:
      return true;
    default:
      S67_CORE_ASSERT(false, "Invalid layer");
      return false;
    }
  }
};

class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
public:
  virtual bool ShouldCollide(JPH::ObjectLayer inObject1,
                             JPH::ObjectLayer inObject2) const override {
    switch (inObject1) {
    case Layers::NON_MOVING:
      return inObject2 == Layers::MOVING;
    case Layers::MOVING:
      return true;
    default:
      S67_CORE_ASSERT(false, "Invalid layer");
      return false;
    }
  }
};

// --- PhysicsSystem Implementation ---

JPH::PhysicsSystem *PhysicsSystem::s_PhysicsSystem = nullptr;
JPH::TempAllocatorImpl *PhysicsSystem::s_TempAllocator = nullptr;
JPH::JobSystemThreadPool *PhysicsSystem::s_JobSystem = nullptr;

static BPLayerInterfaceImpl s_BPLayerInterface;
static ObjectVsBroadPhaseLayerFilterImpl s_ObjectVsBPFilter;
static ObjectLayerPairFilterImpl s_ObjectLayerPairFilter;

void PhysicsSystem::Init() {
  JPH::RegisterDefaultAllocator();
  JPH::Factory::sInstance = new JPH::Factory();
  JPH::RegisterTypes();

  s_TempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
  s_JobSystem = new JPH::JobSystemThreadPool(
      JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers,
      JPH::thread::hardware_concurrency() - 1);

  s_PhysicsSystem = new JPH::PhysicsSystem();
  s_PhysicsSystem->Init(1024, 0, 1024, 1024, s_BPLayerInterface,
                        s_ObjectVsBPFilter, s_ObjectLayerPairFilter);

  s_PhysicsSystem->Init(1024, 0, 1024, 1024, s_BPLayerInterface,
                        s_ObjectVsBPFilter, s_ObjectLayerPairFilter);

  // Apply initial gravity from ConVar
  SetGravity(sv_gravity.GetFloat());

  S67_CORE_INFO("Physics System Initialized (Jolt)");
}

void PhysicsSystem::SetGravity(float gravity) {
  if (s_PhysicsSystem) {
    // Jolt uses Y-up by default?
    // Our scene is Y-up. Gravity is usually negative Y.
    // Source uses positive number for magnitude usually, but vector is
    // (0,0,-800) in source? Wait, Source is Z-up. S67 appears to be Y-up based
    // on floor at -2.0f and Player at 1.7f. So gravity should be (0, -gravity,
    // 0)

    // Standard Source sv_gravity is 800.
    s_PhysicsSystem->SetGravity(JPH::Vec3(0, -gravity, 0));
  }
}

void PhysicsSystem::Shutdown() {
  delete s_PhysicsSystem;
  delete s_JobSystem;
  delete s_TempAllocator;
  delete JPH::Factory::sInstance;
  JPH::Factory::sInstance = nullptr;
}

void PhysicsSystem::OnUpdate(Timestep ts) {
  s_PhysicsAccumulator += ts.GetSeconds();

  int steps = 0;
  while (s_PhysicsAccumulator >= FIXED_PHYSICS_DT &&
         steps < MAX_PHYSICS_STEPS) {
    s_PhysicsSystem->Update(FIXED_PHYSICS_DT, 1, s_TempAllocator, s_JobSystem);
    s_PhysicsAccumulator -= FIXED_PHYSICS_DT;
    steps++;
  }

  // Prevent spiral of death
  if (s_PhysicsAccumulator > FIXED_PHYSICS_DT * MAX_PHYSICS_STEPS) {
    s_PhysicsAccumulator = FIXED_PHYSICS_DT;
  }
}

JPH::BodyID PhysicsSystem::Raycast(const glm::vec3 &origin,
                                   const glm::vec3 &direction, float distance) {
  JPH::RRayCast ray(JPH::RVec3(origin.x, origin.y, origin.z),
                    JPH::Vec3(direction.x * distance, direction.y * distance,
                              direction.z * distance));
  JPH::RayCastResult result;

  if (s_PhysicsSystem->GetNarrowPhaseQuery().CastRay(ray, result)) {
    return result.mBodyID;
  }

  return JPH::BodyID();
}

// --- Default Filters ---
class DefaultBroadPhaseLayerFilter : public JPH::BroadPhaseLayerFilter {
public:
  virtual bool ShouldCollide(JPH::BroadPhaseLayer inLayer) const override {
    return true;
  }
};

class DefaultObjectLayerFilter : public JPH::ObjectLayerFilter {
public:
  virtual bool ShouldCollide(JPH::ObjectLayer inLayer) const override {
    return true;
  }
};

static DefaultBroadPhaseLayerFilter s_DefaultBroadPhaseLayerFilter;
static DefaultObjectLayerFilter s_DefaultObjectLayerFilter;

const JPH::BroadPhaseLayerFilter &PhysicsSystem::GetBroadPhaseLayerFilter() {
  return s_DefaultBroadPhaseLayerFilter;
}

const JPH::ObjectLayerFilter &PhysicsSystem::GetObjectLayerFilter() {
  return s_DefaultObjectLayerFilter;
}

} // namespace S67
