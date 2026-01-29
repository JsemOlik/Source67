# Source Engine Movement System Implementation Prompt

You are implementing a 1:1 clone of the Source Engine (Half-Life 2) movement system in C++. This is a complete specification document with all necessary technical details. Implement exactly as specified.

## PROJECT REQUIREMENTS

**Target**: Exact replication of Source Engine player movement mechanics including walking, running, crouching, jumping, air strafing, and velocity physics.

**Language**: C++
**Framework**: (Your engine of choice - ensure coordinate system compatibility)

---

## UNIT SYSTEM

Source Engine uses **Hammer Units (HU)**. Use this throughout:

- 1 Hammer Unit ≈ 0.025 meters
- 39.97 HU = 1 meter
- All calculations must use HU internally

Conversion for display/graphics:

```cpp
// HU to meters (if needed)
float hu_to_meters = 1.0f / 39.97f;
float velocity_meters = velocity_hu * hu_to_meters;
```

---

## MOVEMENT SPEED CONSTANTS

Define these as class members or constants:

```cpp
// Ground Movement Speeds (HU/s)
const float SPEED_CROUCH = 63.3f;      // Crouching max speed
const float SPEED_WALK = 150.0f;       // Alt+move (slower controlled movement)
const float SPEED_RUN = 190.0f;        // Normal forward movement
const float SPEED_SPRINT = 320.0f;     // Sprint (shift, 8 sec max)

// Acceleration Constants
const float SV_ACCELERATE = 5.6f;      // Ground acceleration (HU/s²)
const float SV_AIRACCELERATE = 12.0f;  // Air acceleration (HU/s²)
const float SV_FRICTION = 4.8f;        // Ground friction/drag

// Air Strafing Specific
const float MAX_AIR_WISH_SPEED = 30.0f;   // Cap on wish direction in air
const float MAX_AIR_SPEED_CAP = 30.0f;    // Limits addspeed before reduction

// Jump
const float JUMP_VELOCITY = 268.0f;    // Upward velocity on jump (HU/s)

// Movement Input Speeds (for conversion from key input)
const float CL_FORWARD_SPEED = 450.0f;  // Forward movement speed
const float CL_BACK_SPEED = 450.0f;     // Back movement speed
const float CL_SIDE_SPEED = 450.0f;     // Strafe left/right speed

// Gravity
const float GRAVITY = 800.0f;           // Downward acceleration (HU/s²)

// Sprint
const float SPRINT_MAX_DISTANCE = 2560.0f;  // Max horizontal distance while sprinting
const float SPRINT_DURATION = 8.0f;        // Max sprint duration (seconds)
const float SPRINT_RECOVERY = 8.0f;        // Recovery time before can sprint again
```

---

## PLAYER STATE STRUCTURE

Create a structure to hold movement state:

```cpp
struct PlayerMoveState {
    // Velocity (3D vector: X=right, Y=up, Z=forward in HL2 convention)
    glm::vec3 velocity;

    // Position
    glm::vec3 position;

    // Camera/View angles
    float yaw;      // Horizontal rotation
    float pitch;    // Vertical rotation (looking up/down)

    // Movement flags
    bool is_grounded;
    bool is_sprinting;
    float sprint_remaining;      // Time left in current sprint
    float sprint_recovery_time;  // Time until can sprint again

    // Crouch state
    bool is_crouching;
    float crouch_transition;     // 0-1 for animation blending

    // Input state (current frame)
    float forward_input;         // -1, 0, or 1
    float side_input;           // -1, 0, or 1
    bool jump_pressed;
    bool sprint_pressed;
    bool crouch_pressed;

    // Surface info
    float surface_friction;      // 1.0 default, varies by surface
};
```

---

## CORE MOVEMENT TICK

Every physics frame, execute in this order:

```cpp
void UpdatePlayerMovement(PlayerMoveState& state, float deltaTime) {
    // 1. Apply friction (ground only)
    if (state.is_grounded) {
        ApplyFriction(state, deltaTime);
    }

    // 2. Read input (from input system)
    // state.forward_input, state.side_input already set by input handler

    // 3. Calculate wish direction and speed
    glm::vec3 wish_dir;
    float wish_speed;
    CalculateWishDirection(state, wish_dir, wish_speed);

    // 4. Apply movement acceleration
    if (state.is_grounded) {
        GroundMove(state, wish_dir, wish_speed, deltaTime);
    } else {
        AirMove(state, wish_dir, wish_speed, deltaTime);
    }

    // 5. Apply gravity (always, except on slopes/ground)
    if (!state.is_grounded) {
        state.velocity.y -= GRAVITY * deltaTime;
    }

    // 6. Move player and detect ground collision
    MoveAndCollide(state, deltaTime);

    // 7. Handle jump input
    if (state.jump_pressed && state.is_grounded) {
        PerformJump(state);
        state.jump_pressed = false;  // Consume input
    }
}
```

---

## FRICTION CALCULATION (Ground Only)

Apply exponential decay:

```cpp
void ApplyFriction(PlayerMoveState& state, float deltaTime) {
    // Friction only applies on ground
    if (!state.is_grounded) return;

    float speed = glm::length(state.velocity);
    if (speed <= 0.01f) {
        state.velocity = glm::vec3(0.0f);  // Stop very small velocities
        return;
    }

    // Friction formula: v *= (1 - friction * friction_multiplier * dt)
    float friction_factor = 1.0f - (SV_FRICTION * state.surface_friction * deltaTime);
    friction_factor = glm::max(friction_factor, 0.0f);

    state.velocity *= friction_factor;

    // Clean up floating point noise
    if (glm::length(state.velocity) < 0.01f) {
        state.velocity = glm::vec3(0.0f);
    }
}
```

---

## WISH DIRECTION CALCULATION

Calculate desired movement direction from input:

```cpp
void CalculateWishDirection(PlayerMoveState& state, glm::vec3& wish_dir, float& wish_speed) {
    // Get forward and right vectors from camera angle
    glm::vec3 forward_vec = GetForwardVector(state.yaw, state.pitch);
    glm::vec3 right_vec = GetRightVector(state.yaw);

    // Create wish velocity from input
    // Note: Constrain pitch to prevent vertical movement input
    glm::vec3 forward_flat = GetForwardVector(state.yaw, 0.0f);  // Flatten pitch

    glm::vec3 wish_vel = (forward_flat * state.forward_input) +
                         (right_vec * state.side_input);

    // Determine max speed based on state
    float max_speed = SPEED_RUN;
    if (state.is_sprinting && state.sprint_remaining > 0) {
        max_speed = SPEED_SPRINT;
    } else if (state.is_crouching) {
        max_speed = SPEED_CROUCH;
    }

    // Calculate wish direction and speed
    wish_speed = glm::length(wish_vel);
    if (wish_speed > 0.01f) {
        wish_dir = glm::normalize(wish_vel);
        // Cap wish speed to max for this state
        wish_speed = glm::min(wish_speed, max_speed);
    } else {
        wish_dir = glm::vec3(0.0f);
        wish_speed = 0.0f;
    }
}
```

---

## GROUND MOVEMENT (Walking/Running)

```cpp
void GroundMove(PlayerMoveState& state, const glm::vec3& wish_dir,
                float wish_speed, float deltaTime) {
    // Standard acceleration on ground
    Accelerate(state.velocity, wish_dir, wish_speed, SV_ACCELERATE,
               state.surface_friction, deltaTime);

    // Cap velocity to max ground speed (unless exceeding, then friction handles it)
    // On ground, velocity.y is typically 0 (unless jumping just happened)
}

void Accelerate(glm::vec3& velocity, const glm::vec3& wish_dir, float wish_speed,
                float accel_constant, float friction_multiplier, float deltaTime) {
    // Project current velocity onto wish direction
    float current_speed = glm::dot(velocity, wish_dir);

    // Calculate how much we can accelerate
    float add_speed = wish_speed - current_speed;

    // If already moving at or beyond wish speed in wish direction, no acceleration
    if (add_speed <= 0.0f) {
        return;
    }

    // Calculate acceleration for this frame
    float accel_speed = accel_constant * deltaTime * wish_speed * friction_multiplier;

    // Cap acceleration to not exceed what we can add
    accel_speed = glm::min(accel_speed, add_speed);

    // Apply acceleration
    velocity += accel_speed * wish_dir;
}
```

---

## AIR MOVEMENT (Air Strafing)

This is the critical section for 1:1 accuracy:

```cpp
void AirMove(PlayerMoveState& state, const glm::vec3& wish_dir,
             float wish_speed, float deltaTime) {
    // In air, use air strafing acceleration
    AirAccelerate(state.velocity, wish_dir, wish_speed, SV_AIRACCELERATE, deltaTime);
}

void AirAccelerate(glm::vec3& velocity, const glm::vec3& wish_dir,
                   float wish_speed, float airaccel, float deltaTime) {
    // Clamp wish speed to air maximum (THIS IS KEY FOR STRAFING)
    if (wish_speed > MAX_AIR_WISH_SPEED) {
        wish_speed = MAX_AIR_WISH_SPEED;  // 30 HU/s
    }

    // Project current velocity onto wish direction (2D on XZ plane, ignore Y)
    // CRITICAL: Use only XZ components (horizontal) for strafing
    glm::vec3 vel_flat = glm::vec3(velocity.x, 0.0f, velocity.z);
    float current_speed = glm::dot(vel_flat, wish_dir);

    // Calculate how much we can accelerate
    float add_speed = wish_speed - current_speed;

    // If adding speed would not help, stop
    if (add_speed <= 0.0f) {
        return;
    }

    // Calculate acceleration for this frame
    // accelspeed = airaccelerate * frametime * wishspeed
    float accel_speed = airaccel * deltaTime * wish_speed;

    // Cap acceleration to not exceed what we can add
    if (accel_speed > add_speed) {
        accel_speed = add_speed;
    }

    // Apply acceleration (only horizontal component)
    // Keep vertical velocity unchanged (gravity handles it)
    velocity.x += accel_speed * wish_dir.x;
    velocity.z += accel_speed * wish_dir.z;
    // Y velocity (vertical) is NOT modified by air acceleration
}
```

**WHY THIS ENABLES STRAFING:**

- When moving perpendicular (90°) to velocity: `dot(velocity, wish_dir) ≈ 0`
- Therefore: `add_speed = 30 - 0 = 30` (maximum possible)
- Player can then accelerate at maximum rate per frame
- By continuously adjusting direction, player gains speed unboundedly
- Friction on landing will slow them down until next jump

---

## JUMP MECHANICS

```cpp
void PerformJump(PlayerMoveState& state) {
    // Set vertical velocity to jump strength
    state.velocity.y = JUMP_VELOCITY;

    // Mark as airborne (might already be true, but ensure)
    state.is_grounded = false;

    // Note: Horizontal velocity carries through jump
    // This is why circle strafing + jump gives initial speed boost
}
```

---

## CROUCH MECHANICS

```cpp
void UpdateCrouch(PlayerMoveState& state, float deltaTime) {
    // Track crouch state transitions
    if (state.crouch_pressed && !state.is_crouching) {
        state.is_crouching = true;
        // Animation transition time (typically 0.2 seconds)
        state.crouch_transition = 0.0f;
    } else if (!state.crouch_pressed && state.is_crouching) {
        state.is_crouching = false;
        state.crouch_transition = 0.0f;
    }

    // Update transition
    if (state.crouch_transition < 1.0f) {
        state.crouch_transition += deltaTime / 0.2f;  // 0.2s animation
        state.crouch_transition = glm::min(state.crouch_transition, 1.0f);
    }

    // Update eye height (for camera)
    float eye_height = glm::mix(64.0f, 28.0f, state.crouch_transition);
    // Apply eye_height to camera position
}
```

---

## COLLISION DETECTION & GROUND DETECTION

```cpp
void MoveAndCollide(PlayerMoveState& state, float deltaTime) {
    // Move player by velocity
    glm::vec3 new_position = state.position + state.velocity * deltaTime;

    // Perform collision checks (raycast/sweep test)
    // This is engine-dependent, but general approach:
    CollisionResult collision = CheckCollision(state.position, new_position);

    if (collision.hit) {
        // Stop at collision point
        new_position = collision.contact_point;

        // Slide along surface (simplified)
        state.velocity = glm::reflect(state.velocity, collision.surface_normal);
        state.velocity *= 0.9f;  // Energy loss
    }

    state.position = new_position;

    // Ground detection: raycast downward
    float ground_check_distance = 2.0f;  // HU
    glm::vec3 ground_check_point = state.position + glm::vec3(0, -ground_check_distance, 0);

    if (CheckCollision(state.position, ground_check_point).hit) {
        state.is_grounded = true;
        state.velocity.y = 0.0f;  // Stop downward velocity on ground
    } else {
        state.is_grounded = false;
    }
}
```

---

## SPRINT MECHANICS

```cpp
void UpdateSprint(PlayerMoveState& state, float deltaTime) {
    if (state.sprint_pressed && !state.is_sprinting &&
        state.sprint_recovery_time <= 0.0f && state.is_grounded) {
        // Start sprinting
        state.is_sprinting = true;
        state.sprint_remaining = SPRINT_DURATION;
        state.sprint_recovery_time = 0.0f;
    }

    if (state.is_sprinting) {
        state.sprint_remaining -= deltaTime;
        if (state.sprint_remaining <= 0.0f) {
            // Sprint expired
            state.is_sprinting = false;
            state.sprint_recovery_time = SPRINT_RECOVERY;
        }
    } else if (state.sprint_recovery_time > 0.0f) {
        state.sprint_recovery_time -= deltaTime;
    }

    // Note: Sprinting doesn't prevent other movement, it just increases max_speed
}
```

---

## ADVANCED TECHNIQUES (OPTIONAL VALIDATION)

These should naturally emerge from the implementation above:

### Bunnyhopping

- Jump at frame-perfect timing (0 ms after landing)
- Strafing accelerates player in air
- Landing resets ground friction, then immediately jump again
- No special code needed; natural consequence of jump + air strafing

### Accelerated Back Hopping (ABH)

- Look backward while jumping
- Wish direction points backward (negative)
- Strafe side-to-side while airborne
- When backward-moving velocity is "negative," acceleration applies as backward boost
- Natural consequence of dot product math

### Circle Strafing

- Sprint + strafe left/right
- Turn camera same direction as strafe
- Jump when reaching desired speed
- Momentum carries to air

### Surface Friction Variation

- Different surfaces (wood, metal, ice) have different friction multipliers
- Apply through surface_friction variable (0.5-1.0)

---

## TESTING CHECKLIST

Validate implementation against these real Source Engine behaviors:

- [ ] Standing still decelerates to 0 in ~1 second
- [ ] Running forward reaches 190 HU/s max
- [ ] Sprinting reaches 320 HU/s max (8 second duration)
- [ ] Jumping adds 268 HU/s upward velocity instantly
- [ ] Gravity of 800 HU/s² applied downward in air
- [ ] Air strafing can accelerate beyond 320 HU/s (key test!)
- [ ] Perpendicular strafing (90° to velocity) accelerates fastest
- [ ] Parallel strafing (forward) doesn't accelerate in air
- [ ] Backward strafing can reverse direction while accelerating
- [ ] Crouching max speed is 63.3 HU/s
- [ ] Crouch animation takes ~0.2 seconds
- [ ] Landing resets ground contact and applies friction next tick
- [ ] Crouch jumping preserves momentum
- [ ] Bunnyhopping accelerates with perfect timing
- [ ] ABH works: backward jump + strafe backwards = forward acceleration
- [ ] No velocity cap in air (except soft 30 HU/s wish speed cap)
- [ ] Hard velocity cap on ground naturally enforces via friction

---

## COORDINATE SYSTEM NOTES

Source Engine standard:

- **X axis**: Right (positive right, negative left)
- **Y axis**: Up (positive up, negative down)
- **Z axis**: Forward (positive forward, negative back)

Ensure your implementation matches this or adjust wish_dir calculations accordingly.

---

## COMMON IMPLEMENTATION MISTAKES TO AVOID

1. **❌ Clamping velocity directly in air** → Breaks strafing
   - ✅ Let velocity grow unbounded, only limit wish_speed
2. **❌ Including vertical velocity in air strafe calculation** → Physics wrong
   - ✅ Use only XZ (horizontal) plane for strafe math
3. **❌ Applying friction in air** → Unrealistic, kills strafing
   - ✅ Friction only on ground
4. **❌ Not preserving horizontal momentum on jump** → Lose speed
   - ✅ Jump only affects Y, preserves X and Z
5. **❌ Forgetting air speed cap of 30 HU/s** → Too fast acceleration
   - ✅ Cap wish_speed to 30 in air acceleration function
6. **❌ Linear friction instead of exponential** → Deceleration feels wrong
   - ✅ Use `v *= (1 - friction * dt)`
7. **❌ Applying acceleration before friction** → Wrong order
   - ✅ Always friction first, then acceleration
8. **❌ Not checking ground status properly** → Stuck in air or rolling
   - ✅ Raycast down each frame, set grounded flag
9. **❌ Jump velocity scaling with framerate** → Inconsistent jumps
   - ✅ Apply instant velocity, don't multiply by deltaTime
10. **❌ Sprint consuming stamina in unusual ways** → Wrong feel
    - ✅ Simple timer: 8 sec max, 8 sec recovery, need all 3

---

## FINAL NOTES

- **Framerate Dependency**: Optimal strafe angle varies with tickrate (30 Hz vs 60 Hz vs 125 Hz)
- **Floating Point**: Use float throughout, especially for deltaTime
- **Source Code Reference**: Original Quake 1 AirAccelerate function in sv_user.c
- **Testing Reference**: Project Borealis (Unreal Engine) implements this identically
- **Community Testing**: Verified accurate by Half-Life 2 speedrunners (DeSinc, spikehunter)

This specification should produce movement that feels exactly like Source Engine / Half-Life 2. Every detail matters for 1:1 accuracy.
