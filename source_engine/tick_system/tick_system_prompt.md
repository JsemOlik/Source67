# Source Engine Tick System Implementation Prompt

You are implementing the tick system used by the Source Engine. This system decouples rendering from physics simulation, ensuring framerate-independent and deterministic gameplay. This is a CRITICAL system that must work perfectly with your movement code.

## OVERVIEW

The tick system solves the problem: **"Why does my game play differently at 30 FPS vs 400 FPS?"**

The answer: **Separate physics ticks (fixed) from rendering frames (variable).**

Every physics simulation happens in discrete, fixed-duration time steps called "ticks." Rendering can happen at any framerate, and interpolation smooths the visuals between physics states.

---

## CORE CONSTANTS

Define these at the top of your engine:

```cpp
// Tick System Configuration
const float TICK_RATE = 66.0f;              // Hz (ticks per second)
const float TICK_DURATION = 1.0f / 66.0f;   // ~0.015151515f seconds (15.15ms)
const float MAX_FRAME_TIME = 0.25f;         // Max 250ms per frame (prevents spiral of death)

// For reference:
// 66 Hz = 15.15ms per tick
// 60 Hz = 16.67ms per tick
// 100 Hz = 10.0ms per tick
// 64 Hz = 15.625ms per tick (CS:GO standard)
```

---

## GAME STATE STRUCTURE

Create a structure to hold all physics state for interpolation:

```cpp
struct GameState {
    // Position and rotation
    glm::vec3 player_position;
    glm::vec3 player_velocity;
    float yaw;      // Horizontal rotation
    float pitch;    // Vertical rotation

    // Movement state
    bool is_grounded;
    bool is_sprinting;
    float sprint_remaining;
    float sprint_recovery_time;
    bool is_crouching;
    float crouch_transition;

    // Input state (sampled in this tick)
    float forward_input;      // -1, 0, 1
    float side_input;         // -1, 0, 1
    bool jump_pressed;
    bool sprint_pressed;
    bool crouch_pressed;

    // Camera/eye height
    float eye_height;

    // Other entities (if using ECS, store entity list)
    // std::vector<Entity> entities;
};
```

---

## MAIN GAME LOOP

This is the core heartbeat of your engine:

```cpp
class GameEngine {
private:
    GameState current_state;
    GameState previous_state;

    double accumulator = 0.0;
    double previous_frame_time = GetCurrentTime();

public:
    void Run() {
        while (IsRunning()) {
            MainLoop();
        }
    }

private:
    void MainLoop() {
        // PHASE 1: Measure frame time
        double current_frame_time = GetCurrentTime();
        double frame_time = current_frame_time - previous_frame_time;
        previous_frame_time = current_frame_time;

        // PHASE 2: Prevent spiral of death (lag spike safety)
        if (frame_time > MAX_FRAME_TIME) {
            frame_time = MAX_FRAME_TIME;  // Clamp to max 250ms
        }

        // PHASE 3: Accumulate real time
        accumulator += frame_time;

        // PHASE 4: Process all due physics ticks
        int tick_count = 0;
        while (accumulator >= TICK_DURATION) {
            // Save previous state for interpolation
            previous_state = current_state;

            // Run one physics tick with fixed delta time
            UpdateGameTick(current_state, TICK_DURATION);

            // Deduct from accumulator
            accumulator -= TICK_DURATION;
            tick_count++;
        }

        // PHASE 5: Render frame with interpolation
        float alpha = (float)(accumulator / TICK_DURATION);
        RenderFrame(alpha);

        // PHASE 6: Optional debugging
        if (show_debug_info) {
            float fps = 1.0f / (float)frame_time;
            printf("FPS: %.1f, Ticks/Frame: %d, Interp: %.3f\n", fps, tick_count, alpha);
        }
    }
};
```

---

## PHYSICS TICK UPDATE

This function runs at exactly 66 Hz (or whatever tickrate you choose):

```cpp
void UpdateGameTick(GameState& state, float tick_dt) {
    // CRITICAL: Always use tick_dt, NEVER variable frame delta time

    // 1. Sample input (keyboard, mouse state at THIS tick)
    SampleInput(state);

    // 2. Update camera/view direction
    UpdateViewAngles(state);

    // 3. Update crouch state and animation
    UpdateCrouch(state, tick_dt);

    // 4. Update sprint state
    UpdateSprint(state, tick_dt);

    // 5. Apply movement and physics (use your movement code here!)
    ApplyMovement(state, tick_dt);

    // 6. Collision detection and ground contact
    CheckCollisionAndGround(state);

    // 7. Apply gravity (if in air)
    if (!state.is_grounded) {
        state.player_velocity.y -= GRAVITY * tick_dt;
    }

    // 8. Handle jump input
    if (state.jump_pressed && state.is_grounded) {
        PerformJump(state);
        state.jump_pressed = false;  // Consume input
    }

    // 9. Clamp excessive velocities (optional safety)
    float max_velocity = 1000.0f;
    if (glm::length(state.player_velocity) > max_velocity) {
        state.player_velocity = glm::normalize(state.player_velocity) * max_velocity;
    }
}
```

---

## MOVEMENT CODE INTEGRATION

Your movement functions (from the movement prompt) should accept tick_dt:

```cpp
void ApplyMovement(GameState& state, float tick_dt) {
    // Pass tick_dt instead of variable frame delta time

    if (state.is_grounded) {
        ApplyFriction(state, tick_dt);
        GroundMove(state, tick_dt);
    } else {
        AirMove(state, tick_dt);
    }
}

// Example from movement system:
void ApplyFriction(GameState& state, float tick_dt) {
    if (!state.is_grounded) return;

    // velocity *= (1 - friction * friction_multiplier * dt)
    float friction_factor = 1.0f - (SV_FRICTION * 1.0f * tick_dt);
    friction_factor = glm::max(friction_factor, 0.0f);

    state.player_velocity *= friction_factor;
}
```

**KEY DIFFERENCE FROM BEFORE:**

- Old: `velocity *= (1 - friction * dt)` where `dt` is frame-dependent
- New: `velocity *= (1 - friction * TICK_DURATION)` where TICK_DURATION is always 0.015151515f

---

## INTERPOLATION FOR RENDERING

This ensures smooth visuals between physics ticks:

```cpp
void RenderFrame(float alpha) {
    // alpha is 0.0 to ~0.999
    // 0.0 = at previous tick state
    // 0.5 = halfway between previous and current tick
    // 0.999 = almost at current tick

    // Interpolate position
    glm::vec3 display_position = glm::mix(
        previous_state.player_position,
        current_state.player_position,
        alpha
    );

    // Interpolate rotation
    float display_yaw = glm::mix(
        previous_state.yaw,
        current_state.yaw,
        alpha
    );
    float display_pitch = glm::mix(
        previous_state.pitch,
        current_state.pitch,
        alpha
    );

    // Interpolate eye height (for crouch animation)
    float display_eye_height = glm::mix(
        previous_state.eye_height,
        current_state.eye_height,
        alpha
    );

    // Set camera
    glm::vec3 camera_pos = display_position + glm::vec3(0, display_eye_height, 0);
    glm::mat4 view = GetViewMatrix(camera_pos, display_yaw, display_pitch);

    // Render all entities with interpolation
    for (const auto& entity : current_state.entities) {
        glm::vec3 entity_display_pos = glm::mix(
            previous_state.entities[entity.id].position,
            entity.position,
            alpha
        );
        RenderEntity(entity, entity_display_pos);
    }

    // Render world geometry, UI, etc.
    RenderWorld(view);
    RenderUI();

    // Swap buffers
    SwapBuffers();
}
```

---

## INPUT HANDLING

Input must be sampled during the tick, not the frame:

```cpp
// Global or class member: track current input state
struct InputState {
    bool key_w, key_a, key_s, key_d;
    bool key_space;
    bool key_shift;
    bool key_ctrl;
    float mouse_delta_x, mouse_delta_y;
} current_input;

// Called every frame (from OS/GLFW/SDL event loop)
void HandleInputEvent(InputEvent event) {
    if (event.type == KEY_DOWN) {
        if (event.key == 'W') current_input.key_w = true;
        if (event.key == 'A') current_input.key_a = true;
        // ... etc for all keys
    }
    if (event.type == KEY_UP) {
        if (event.key == 'W') current_input.key_w = false;
        // ... etc
    }
    if (event.type == MOUSE_MOVE) {
        current_input.mouse_delta_x += event.delta_x;
        current_input.mouse_delta_y += event.delta_y;
    }
}

// Called during tick update
void SampleInput(GameState& state) {
    // Convert key states to movement input
    state.forward_input = 0.0f;
    if (current_input.key_w) state.forward_input += 1.0f;
    if (current_input.key_s) state.forward_input -= 1.0f;

    state.side_input = 0.0f;
    if (current_input.key_d) state.side_input += 1.0f;
    if (current_input.key_a) state.side_input -= 1.0f;

    // Consume mouse look
    state.yaw += current_input.mouse_delta_x * MOUSE_SENSITIVITY;
    state.pitch -= current_input.mouse_delta_y * MOUSE_SENSITIVITY;

    // Clamp pitch
    state.pitch = glm::clamp(state.pitch, -89.0f, 89.0f);

    // Clear accumulated mouse delta
    current_input.mouse_delta_x = 0.0f;
    current_input.mouse_delta_y = 0.0f;

    // Sample button inputs
    state.jump_pressed = current_input.key_space;
    state.sprint_pressed = current_input.key_shift;
    state.crouch_pressed = current_input.key_ctrl;
}
```

---

## FIXED TIMESTEP BENEFITS FOR MOVEMENT

Your air strafing and movement system now works correctly:

**Before (Variable DT):**

```
At 30 FPS (dt=33.33ms):
  Air acceleration = 12 * 0.03333 * 30 = 12 HU per frame
  30 frames/sec * 12 = 360 HU/sec total

At 400 FPS (dt=2.5ms):
  Air acceleration = 12 * 0.0025 * 30 = 0.9 HU per frame
  400 frames/sec * 0.9 = 360 HU/sec total

Sound same on paper, but accumulation errors cause differences!
```

**After (Fixed Tick):**

```
Every tick (dt=15.15ms):
  Air acceleration = 12 * 0.015151515 * 30 = 5.45 HU per tick
  66 ticks/sec * 5.45 = 359.7 HU/sec total

EXACTLY SAME RESULT whether you render at 30 FPS or 400 FPS!
```

---

## NETWORK SYNCHRONIZATION

When implementing multiplayer:

```cpp
// Server side
void UpdateServer() {
    while (accumulator >= TICK_DURATION) {
        // Update all players with their input
        for (auto& player : players) {
            UpdateGameTick(player.state, TICK_DURATION);
        }

        // Take snapshot (every tick or every N ticks)
        if (tick_number % snapshot_interval == 0) {
            TakeSnapshot();
        }

        accumulator -= TICK_DURATION;
    }
}

// Client side
void UpdateClient() {
    while (accumulator >= TICK_DURATION) {
        // Update local player based on input
        UpdateGameTick(local_player, TICK_DURATION);

        // Update other players based on last received snapshot + prediction
        for (auto& other_player : remote_players) {
            // Option 1: Interpolate to last snapshot
            // Option 2: Predict based on last velocity
        }

        accumulator -= TICK_DURATION;
    }
}
```

---

## SPIRAL OF DEATH PREVENTION

If your computer has a lag spike and can't render for 500ms:

**Without frame_time clamping:**

```
Frame time = 500ms
accumulator += 0.5
0.5 / 0.015 = 33 ticks to process
All 33 ticks execute instantly
That takes even longer
CPU can't catch up
Frame time next frame = 1000ms
Spiral down → crash
```

**With frame_time clamping:**

```
Frame time = 500ms → clamped to 250ms
accumulator += 0.25
0.25 / 0.015 = 16 ticks to process
16 ticks execute quickly
accumulator still has 0.1s worth
Next frame processes remaining
Game recovers gracefully
```

**Your code:**

```cpp
if (frame_time > MAX_FRAME_TIME) {
    frame_time = MAX_FRAME_TIME;  // 0.25 seconds
}
```

---

## DEBUGGING & VERIFICATION

Add these helper functions:

```cpp
void PrintTickStats() {
    static float time_since_last_print = 0;
    time_since_last_print += TICK_DURATION;

    if (time_since_last_print >= 1.0f) {
        printf("Tick %d: Position (%.2f, %.2f, %.2f)\n",
            tick_number,
            current_state.player_position.x,
            current_state.player_position.y,
            current_state.player_position.z);
        time_since_last_print = 0;
    }
}

void VerifyFramerateIndependence() {
    // Same input sequence should produce same output
    // regardless of whether you render at 30, 60, or 300 FPS

    // Test: Move forward for exactly 100 ticks
    // Measure final position
    // Should be identical if you run at different framerates
}

float GetCurrentTickNumber() {
    // For debugging: which tick are we in?
    return 1.0 / TICK_DURATION * accumulated_time;
}

float GetInterpolationAlpha() {
    return (float)(accumulator / TICK_DURATION);
}
```

---

## GOTCHAS & COMMON MISTAKES

1. **❌ Using frame delta time in physics**
   - Wrong: `velocity += acceleration * frame_dt`
   - Right: `velocity += acceleration * TICK_DURATION`

2. **❌ Not clamping frame_time**
   - Causes spiral of death on lag spikes
   - Always: `if (frame_time > 0.25) frame_time = 0.25;`

3. **❌ Forgetting to interpolate rendering**
   - Game will stutter between ticks
   - Always use alpha in rendering

4. **❌ Processing input in render loop**
   - Should be in tick loop, sampled once per tick
   - One key press might not be processed if input comes between ticks

5. **❌ Accumulating floating point errors**
   - accumulator += frame_time (small error each time)
   - After hours of play, can drift
   - Solution: Use 64-bit double for accumulator, cast to float for tick_dt

6. **❌ Changing tick rate at runtime without testing**
   - Some physics code might have hardcoded assumptions about 66 Hz
   - Test thoroughly if changing from 66 to 100 Hz

7. **❌ Not matching tick rate between server and client**
   - Server at 66 Hz, client at 100 Hz = desyncs
   - Must be identical

---

## PERFORMANCE CONSIDERATIONS

- **66 Hz**: Baseline, ~1.5% CPU per tick
- **100 Hz**: ~1.5x CPU load
- **128 Hz**: ~2x CPU load
- **Unlimited FPS rendering**: Free (GPU limited, uses interpolation)

For singleplayer, you can go higher than 66 Hz if you want more precision (100-200 Hz is safe). For multiplayer, stick with what the server uses.

---

## FINAL CHECKLIST

Before considering this complete:

- [ ] Tick system compiles and runs
- [ ] Accumulator pattern working (processes multiple ticks if needed)
- [ ] Frame time clamping prevents spiral of death
- [ ] Movement code integrated with TICK_DURATION
- [ ] Interpolation applied in rendering
- [ ] Input sampling in tick loop
- [ ] Same input sequence produces same output at 30 FPS and 300 FPS
- [ ] Framerate independent: moving forward same distance regardless of FPS
- [ ] No stutter or jitter in rendering
- [ ] Debug output shows correct tick numbers and alpha values
- [ ] At 1000+ FPS with 66 Hz ticks, still runs smoothly (rendering interpolates)
- [ ] Lag spike doesn't cause spiral of death
