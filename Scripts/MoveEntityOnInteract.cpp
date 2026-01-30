#include "Renderer/ScriptableEntity.h"
#include "Renderer/ScriptRegistry.h"
#include "Core/KeyCodes.h"

namespace S67 {

class MoveEntityOnInteract : public ScriptableEntity {
public:
  void OnUpdate(float ts) override {
    Entity *hit = Raycast(10.0f);

    if (hit && hit->HasTag("Interactable")) {
      SetText("InteractPrompt", "Press F to Move Cube", {0.5f, 0.5f});

      if (IsKeyPressed(S67_KEY_F)) {
        // Try to find a specific cube to move
        Entity *target = FindEntity("Player");
        if (target) {
          Move(target, {0.1f, 0.0f, 0.0f}); // Move 0.1 units on X each tick
        } else {
          // If no specific target, move the one we are looking at!
          Move(hit, {0.1f, 0.0f, 0.0f});
        }
      }
    } else {
      ClearText("InteractPrompt");
    }
  }

private:
  Entity *m_LastHit = nullptr;
};

REGISTER_SCRIPT(MoveEntityOnInteract);

} // namespace S67
