#include "Renderer/ScriptableEntity.h"
#include "Renderer/ScriptRegistry.h"
#include "Core/KeyCodes.h"

namespace S67 {

class InteractableRaycast : public ScriptableEntity {
public:
  void OnUpdate(float ts) override {
    Entity *currentHit = Raycast(10.0f);

    if (currentHit != m_LastHit) {
      if (currentHit && currentHit->HasTag("Interactable")) {
        SetText("Interaction", "[E] Interactable!", {0.5f, 0.1f});
      } else {
        ClearText("Interaction");
      }
      m_LastHit = currentHit;
    }

    if (currentHit && currentHit->HasTag("Interactable")) {
      if (IsKeyPressed(S67_KEY_E)) {
        Move(currentHit, {0.0f, 10.0f, 0.0f});
      }
    }
  }

private:
  Entity *m_LastHit = nullptr;
};

REGISTER_SCRIPT(InteractableRaycast);

} // namespace S67
