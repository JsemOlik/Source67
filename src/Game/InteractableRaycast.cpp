#include "Renderer/ScriptableEntity.h"
#include "Renderer/HUDRenderer.h"
#include "Physics/PhysicsSystem.h"
#include "Core/Input.h"
#include "Core/Application.h"
#include "Renderer/ScriptRegistry.h"
#include <glm/gtc/quaternion.hpp>

namespace S67 {

class InteractableRaycast : public ScriptableEntity {
public:
  void OnUpdate(float ts) override {
    auto& transform = GetTransform();
    
    // The player's camera position and direction
    // In Source67, the camera is usually at entity position + {0, 1.7, 0}
    glm::vec3 origin = transform.Position + glm::vec3(0.0f, 1.7f, 0.0f);
    
    // Calculate direction from rotation (pitch/yaw)
    // Rotation.x is pitch, Rotation.y is yaw
    float pitch = glm::radians(transform.Rotation.x);
    float yaw = glm::radians(transform.Rotation.y - 90.0f); // Adjust for engine convention
    
    glm::vec3 direction;
    direction.x = cos(pitch) * cos(yaw);
    direction.y = sin(pitch);
    direction.z = cos(pitch) * sin(yaw);
    direction = glm::normalize(direction);

    float distance = 10.0f; // 10 meters interaction range
    JPH::BodyID hitBody = PhysicsSystem::Raycast(origin, direction, distance);

    if (!hitBody.IsInvalid()) {
      auto& bodyInterface = PhysicsSystem::GetBodyInterface();
      Entity* hitEntity = (Entity*)bodyInterface.GetUserData(hitBody);
      
      if (hitEntity) {
        bool isInteractable = false;
        for (const auto& tag : hitEntity->Tags) {
          if (tag == "Interactable") {
            isInteractable = true;
            break;
          }
        }

        if (isInteractable) {
          HUDRenderer::QueueString("Interactable!", {1.0f, 1.0f, 0.0f, 1.0f}); // Yellow text
        }
      }
    }
  }
};

REGISTER_SCRIPT(InteractableRaycast);

} // namespace S67
