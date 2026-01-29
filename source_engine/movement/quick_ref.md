# Source Engine Movement - Quick Reference

## Constants (Copy-Paste Ready)

```cpp
// Speeds (HU/s)
const float SPEED_CROUCH = 63.3f;
const float SPEED_WALK = 150.0f;
const float SPEED_RUN = 190.0f;
const float SPEED_SPRINT = 320.0f;

// Acceleration (HU/s²)
const float SV_ACCELERATE = 5.6f;
const float SV_AIRACCELERATE = 12.0f;
const float SV_FRICTION = 4.8f;

// Air Strafing
const float MAX_AIR_WISH_SPEED = 30.0f;
const float MAX_AIR_SPEED_CAP = 30.0f;

// Other
const float JUMP_VELOCITY = 268.0f;
const float GRAVITY = 800.0f;
const float CL_FORWARD_SPEED = 450.0f;
const float CL_SIDE_SPEED = 450.0f;
```

---

## Key Functions (In Order)

### 1. Per-Frame Update

```cpp
ApplyFriction(state, dt);
CalculateWishDirection(state, wish_dir, wish_speed);
if (grounded) GroundMove(...);
else AirMove(...);
ApplyGravity(state, dt);
MoveAndCollide(state, dt);
HandleJump(state);
```

### 2. Friction (Ground Only)

```cpp
velocity *= (1.0f - SV_FRICTION * surface_friction * dt);
```

### 3. Ground Acceleration

```cpp
currentspeed = dot(velocity, wishdir);
addspeed = wish_speed - currentspeed;
if (addspeed <= 0) return;
accelspeed = SV_ACCELERATE * dt * wish_speed * friction;
accelspeed = min(accelspeed, addspeed);
velocity += accelspeed * wishdir;
```

### 4. Air Acceleration (CRITICAL)

```cpp
wish_speed = min(wish_speed, 30.0f);  // Cap to 30!
currentspeed = dot(velocity_xz, wishdir);  // Only XZ!
addspeed = wish_speed - currentspeed;
if (addspeed <= 0) return;
accelspeed = SV_AIRACCELERATE * dt * wish_speed;
accelspeed = min(accelspeed, addspeed);
velocity.xz += accelspeed * wishdir;  // NOT velocity.y
```

---

## Why Air Strafing Works

| Angle               | currentspeed  | addspeed          | Result             |
| ------------------- | ------------- | ----------------- | ------------------ |
| 0° (forward)        | ~velocity     | 30-500 = negative | No acceleration    |
| 45° (diagonal)      | ~velocity/√2  | 30-250 = small    | Some acceleration  |
| 90° (perpendicular) | ~0            | 30-0 = 30         | **MAXIMUM**        |
| 135° (back-ish)     | ~-velocity/√2 | 30+250 = huge     | Fast backward      |
| 180° (backward)     | ~-velocity    | 30+500 = max      | **Backward boost** |

**Key insight**: Perpendicular input = currentspeed near zero = maximum addspeed = fastest acceleration.

---

## Common Issues & Fixes

| Problem                 | Cause                      | Fix                                  |
| ----------------------- | -------------------------- | ------------------------------------ |
| Can't gain speed in air | Clamping velocity          | Remove hard cap; only cap wish_speed |
| Strafing too slow       | Using wrong acceleration   | Use SV_AIRACCELERATE=12, not 5.6     |
| Vertical strafing       | Including Y in strafe math | Use only XZ components               |
| No ground friction      | Friction not applied       | Apply BEFORE acceleration            |
| Jump loses speed        | Not preserving XZ          | Only apply jump to Y velocity        |
| Crouch broken           | Wrong constant             | Use 63.3 HU/s, not 190               |
| Can't stop moving       | Friction calculation wrong | Use exponential: *= (1 - f*dt)       |
| Wrong feel              | Framerate dependent        | Use deltaTime in all calculations    |

---

## Tick Order (EXACT)

1. **Friction** - velocity _= (1 - friction _ dt)
2. **Input** - Read WASD, calculate wish_dir and wish_speed
3. **Accelerate** - Apply ground or air acceleration
4. **Gravity** - velocity.y -= 800 \* dt
5. **Move** - position += velocity \* dt
6. **Collide** - Check collisions, update grounded
7. **Jump** - If grounded + jump pressed: velocity.y = 268

Miss one step = broken physics.

---

## Optimal Strafe Angle

- **At 500 HU/s velocity**: ~88-92° from velocity vector
- **At 300 HU/s velocity**: ~85-90° from velocity vector
- **At 100 HU/s velocity**: ~80-90° from velocity vector
- **Higher velocity** = need more precise angle

Frame rate also matters:

- 30 Hz: Wider optimal angle range
- 60 Hz: Narrower, requires more precision
- 125+ Hz: Very narrow, competitive players need skill

---

## Testing Values

These should match Source Engine 1:1:

| Test               | Expected                   | Tolerance              |
| ------------------ | -------------------------- | ---------------------- |
| Standing, no input | 0 HU/s in ~1 sec           | ±0.1 sec               |
| Walk forward max   | 190 HU/s                   | ±1 HU/s                |
| Sprint max         | 320 HU/s                   | ±1 HU/s                |
| Crouch max         | 63.3 HU/s                  | ±0.5 HU/s              |
| Jump up            | 268 HU/s upward            | ±1 HU/s                |
| Gravity            | Falls 800 HU/s²            | ±10 HU/s²              |
| Air strafe gain    | +2-5 HU/s per frame @ 60Hz | Varies by angle        |
| Bunnyhopping       | ~300+ HU/s with practice   | Player skill dependent |
| ABH max            | 500+ HU/s possible         | High skill required    |

---

## Code Structure Example

```cpp
class PlayerController {
private:
    // State
    glm::vec3 velocity;
    glm::vec3 position;
    bool is_grounded;

    // Input
    float forward_input;
    float side_input;
    bool jump_pressed;

public:
    void Update(float dt) {
        if (is_grounded) ApplyFriction(dt);

        glm::vec3 wish_dir;
        float wish_speed;
        CalculateWish(wish_dir, wish_speed);

        if (is_grounded) {
            Accelerate(wish_dir, wish_speed, SV_ACCELERATE, dt);
        } else {
            AirAccelerate(wish_dir, wish_speed, SV_AIRACCELERATE, dt);
        }

        ApplyGravity(dt);
        MoveAndCollide(dt);
        HandleJump();
    }
};
```

---

## Real-World Values from HL2

- **Max infinite height gain**: 512 HU (with air strafing)
- **Max horizontal distance**: Theoretically infinite (limited by map)
- **Actual speedrun record**: 500+ HU/s achieved
- **Casual player max**: 250-350 HU/s
- **Beginner player max**: 150-200 HU/s

---

## DO's and DON'Ts

### ✅ DO

- Use exponential friction
- Cap wish_speed to 30 in air
- Apply gravity constantly in air
- Preserve XZ momentum on jump
- Check ground every frame
- Use deltaTime in all calculations

### ❌ DON'T

- Clamp velocity in air
- Apply friction in air
- Include vertical in strafe calculations
- Reset momentum on jump
- Use hardcoded numbers for dt
- Forget surface friction multiplier

---

## References

- Original: Quake 1 GPL source (sv_user.c)
- Implemented in: Half-Life, Half-Life 2, Counter-Strike
- Modern: Godot, Unreal Engine, Unity implementations exist
- Testing: Half-Life speedrunning community (validated accurate)

This is battle-tested physics that's 25+ years old. If your numbers don't match, check the tick order first.
