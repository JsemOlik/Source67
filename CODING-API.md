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

### 3. Finding & Manipulating Entities
You can find other objects in the scene by their name and manipulate them.

```cpp
// Find a specific object
Entity* door = FindEntity("MainDoor");

// Move the object (delta)
if (door) Move(door, {0.0f, 5.0f, 0.0f});

// Move the entity this script is on
Move({1.0f, 0.0f, 0.0f});

// Set absolute position
SetPosition({0, 0, 0});

// Rotate (delta in euler degrees)
Rotate({0, 90, 0});
```

### 4. Input Handling
Check if keys are being pressed.
```cpp
if (IsKeyPressed(S67_KEY_E)) {
    // Interaction logic
}
```

### 5. Tags
Easily check if an entity has a specific tag.
```cpp
if (hit->HasTag("Interactable")) {
    // ...
}
```

### 6. Transient HUD Messages
Queue a message that shows up briefly and then disappears.
```cpp
PrintHUD("Achievement Unlocked!", {0.0f, 1.0f, 0.0f, 1.0f}); // Green text
```

## Example: Moving a Cube on Interaction

```cpp
#include "Renderer/ScriptableEntity.h"
#include "Renderer/ScriptRegistry.h"
#include "Core/KeyCodes.h"

namespace S67 {

class DoorOpener : public ScriptableEntity {
public:
    void OnUpdate(float ts) override {
        Entity* hit = Raycast(10.0f);
        
        if (hit && hit->HasTag("Button")) {
            SetText("Prompt", "Press E to open door");
            
            if (IsKeyPressed(S67_KEY_E)) {
                Entity* door = FindEntity("SlidingDoor");
                if (door) Move(door, {0.0f, 0.1f, 0.0f}); // Slide up
            }
        } else {
            ClearText("Prompt");
        }
    }
};

REGISTER_SCRIPT(DoorOpener);

}
```
