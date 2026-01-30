# Source67 Scripting API Guide

Welcome to the Source67 Scripting API! This guide will help you create custom behaviors for your game entities using C++ scripts.

## Getting Started

To create a new script:
1. Create a new `.cpp` file in `src/Game/` (e.g., `MyScript.cpp`).
2. Inherit from `ScriptableEntity`.
3. Register your script using `REGISTER_SCRIPT(ClassName)`.

```cpp
#include "Renderer/ScriptableEntity.h"
#include "Renderer/ScriptRegistry.h"

namespace S67 {

class MyScript : public ScriptableEntity {
public:
    void OnCreate() override {
        // Called when the entity is created or script is attached
    }

    void OnUpdate(float ts) override {
        // Called every physics tick
    }
};

REGISTER_SCRIPT(MyScript);

}
```

## The "Stupid Simple" API

We've added several helper methods to `ScriptableEntity` to make common tasks trivial:

### 1. Raycasting
Perform a standard forward raycast from the player's view.
```cpp
Entity* hit = Raycast(10.0f); // Raycast 10 meters forward
if (hit) {
    // Do something with the hit entity
}
```

### 2. Persistent HUD Text
Display text on the HUD that stays until you clear it. This is perfect for interaction prompts.
```cpp
// Set text with an ID, content, and optional position (normalized 0-1)
SetText("MyID", "Hello World!", {0.5f, 0.5f}); 

// Clear it when done
ClearText("MyID");
```

### 3. Tags
Easily check if an entity has a specific tag.
```cpp
if (hit->HasTag("Interactable")) {
    // ...
}
```

### 4. Transient HUD Messages
Queue a message that shows up briefly and then disappears.
```cpp
PrintHUD("Achievement Unlocked!", {0.0f, 1.0f, 0.0f, 1.0f}); // Green text
```

## Lifecycle Methods

- `OnCreate()`: Initialization logic.
- `OnUpdate(float ts)`: Core logic (runs at fixed physics frequency).
- `OnDestroy()`: Cleanup logic.
- `OnEvent(Event& e)`: Handle engine events (input, window, etc.).

## Accessing Components

You can access the owning entity and its components directly:
- `GetEntity()`: Returns the `Entity` object.
- `GetTransform()`: Shortcut to the entity's `Transform` component.
