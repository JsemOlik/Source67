# Tick System Quick Reference

## Core Formula

```
Real Frame Time (variable)
         ↓
    Accumulator (sum up)
         ↓
    ÷ TICK_DURATION
         ↓
   How many ticks due?
         ↓
   Execute that many ticks
   with fixed dt = TICK_DURATION
         ↓
   Remaining time in accumulator
   becomes interpolation alpha
         ↓
   Render with interpolation
```

---

## Constants (Copy-Paste)

```cpp
const float TICK_RATE = 66.0f;
const float TICK_DURATION = 1.0f / 66.0f;  // 0.015151515f
const float MAX_FRAME_TIME = 0.25f;        // 250ms

// Alternative rates:
// 60 Hz:  1.0f / 60.0f  = 0.016667f
// 100 Hz: 1.0f / 100.0f = 0.010f
// 64 Hz:  1.0f / 64.0f  = 0.015625f (CS:GO)
```

---

## Main Loop Structure

```cpp
double accumulator = 0.0;
double prev_time = now();

while (running) {
    double curr_time = now();
    double dt = curr_time - prev_time;
    prev_time = curr_time;

    // 1. Clamp frame time
    if (dt > 0.25) dt = 0.25;

    // 2. Accumulate
    accumulator += dt;

    // 3. Process ticks
    while (accumulator >= TICK_DURATION) {
        prev_state = curr_state;
        UpdateTick(curr_state, TICK_DURATION);
        accumulator -= TICK_DURATION;
    }

    // 4. Render with interpolation
    float alpha = accumulator / TICK_DURATION;
    Render(alpha);
}
```

---

## Critical Rule

**ALWAYS use TICK_DURATION in physics, NEVER use frame delta time**

```cpp
// ❌ WRONG
position += velocity * frame_dt;

// ✅ CORRECT
position += velocity * TICK_DURATION;
```

---

## Tick vs Frame

| Aspect           | Tick            | Frame                |
| ---------------- | --------------- | -------------------- |
| Timing           | Fixed (15.15ms) | Variable (2ms-100ms) |
| Frequency        | 66 Hz always    | FPS dependent        |
| Physics          | Runs here       | Never                |
| Rendering        | Never           | Always               |
| Count per second | Always 66       | Variable (30-600)    |

---

## Interpolation Pattern

```cpp
Vector3 display_pos = mix(prev_state.pos, curr_state.pos, alpha);
float display_yaw = mix(prev_state.yaw, curr_state.yaw, alpha);

// alpha = 0.0 → show previous tick state
// alpha = 0.5 → show halfway between ticks
// alpha = 0.999 → show almost current tick
```

---

## Frame Time Clamping (Why It Matters)

**Without clamping (BAD):**

```
Lag spike: frame_time = 500ms
accumulator = 500ms
Ticks to process = 500 / 15 = 33 ticks
Processing 33 ticks takes 1000ms
Next frame is even worse
→ SPIRAL OF DEATH
```

**With clamping (GOOD):**

```
Lag spike: frame_time = 500ms → clamped to 250ms
accumulator = 250ms
Ticks to process = 250 / 15 = 16 ticks
Processing 16 ticks takes 50ms
Next frame processes remaining
→ RECOVERY
```

```cpp
if (frame_time > MAX_FRAME_TIME) {
    frame_time = MAX_FRAME_TIME;
}
```

---

## Input Sampling

```cpp
// Every frame (from OS)
void HandleKey(int key, bool pressed) {
    current_input.keys[key] = pressed;
}

// Every tick (in UpdateTick)
void SampleInput(GameState& state) {
    state.forward = (current_input.key_w ? 1 : 0) -
                    (current_input.key_s ? 1 : 0);
    state.side = (current_input.key_d ? 1 : 0) -
                 (current_input.key_a ? 1 : 0);
    state.jump = current_input.key_space;

    // Consume/clear frame input here if needed
}
```

---

## Movement Integration

Replace all variable delta times:

```cpp
// Old (broken)
void ApplyFriction(Vector3& vel, float dt) {
    vel *= (1 - 4.8f * dt);  // dt varies wildly
}

// New (fixed)
void ApplyFriction(Vector3& vel) {
    vel *= (1 - 4.8f * TICK_DURATION);  // Always 0.0727...
}
```

Call in UpdateTick:

```cpp
void UpdateTick(GameState& state, float dt) {
    // dt = TICK_DURATION always

    SampleInput(state);
    if (state.is_grounded) {
        ApplyFriction(state);       // Uses TICK_DURATION internally
        GroundMove(state, dt);
    } else {
        AirMove(state, dt);
    }
    CheckGround(state);
}
```

---

## Tick Numbers

```cpp
// Track which tick we're in
static int tick_number = 0;

void UpdateTick(GameState& state, float dt) {
    // ... tick code ...
    tick_number++;
}

float CurrentTickInSeconds() {
    return tick_number * TICK_DURATION;
}
```

---

## State for Interpolation

```cpp
struct GameState {
    vec3 position;      // Updated every tick
    vec3 velocity;      // Updated every tick
    float yaw, pitch;   // Updated every tick
    // ... movement state ...
};

GameState previous_state, current_state;

// In main loop
while (accumulator >= TICK_DURATION) {
    previous_state = current_state;  // Save for interpolation
    UpdateTick(current_state);       // Update current
    accumulator -= TICK_DURATION;
}

// In render
float alpha = accumulator / TICK_DURATION;
vec3 render_pos = mix(previous_state.pos, current_state.pos, alpha);
```

---

## Checklist

- [ ] TICK_DURATION constant defined
- [ ] Frame time clamped to 0.25s
- [ ] Accumulator pattern in main loop
- [ ] Multiple ticks processed if needed
- [ ] Previous state saved before tick
- [ ] Movement code uses TICK_DURATION
- [ ] Interpolation alpha calculated
- [ ] Rendering uses interpolated values
- [ ] Same input → same output at any FPS
- [ ] No "spiral of death" on lag spikes
- [ ] No stutter in animation
- [ ] Debug prints show tick count and alpha

---

## Framerate Independence Verification

Test this:

```cpp
// Run with 30 FPS cap
// Move forward for 5 seconds (330 ticks)
// Note final position: ~3.17 HU/frame * 150 frames = ~475 HU

// Run with 300 FPS uncapped
// Move forward for 5 seconds (330 ticks)
// Note final position: should be EXACTLY ~475 HU

// Run with 600 FPS (if possible)
// Move forward for 5 seconds (330 ticks)
// Note final position: still EXACTLY ~475 HU

// All three should be IDENTICAL (within floating point error)
```

If they differ:

1. Check that you're using TICK_DURATION, not frame dt
2. Verify frame time clamping is in place
3. Ensure UpdateTick uses tick_dt parameter

---

## Debugging Output

```cpp
void DebugTick() {
    static int frame_count = 0;
    static double print_timer = 0;

    frame_count++;
    print_timer += frame_time;

    if (print_timer >= 1.0) {
        printf("Frame %d | FPS %.1f | Accumulator %.3f | Alpha %.3f | "
               "Ticks %d | Pos (%.1f, %.1f, %.1f)\n",
            frame_count,
            frame_count / print_timer,
            accumulator,
            accumulator / TICK_DURATION,
            tick_number,
            current_state.position.x,
            current_state.position.y,
            current_state.position.z);

        frame_count = 0;
        print_timer = 0;
    }
}
```

Expected output:

```
Frame 60 | FPS 60.0 | Accumulator 0.003 | Alpha 0.200 | Ticks 66 | Pos (150.3, 64.0, 200.5)
Frame 60 | FPS 119.9 | Accumulator 0.008 | Alpha 0.530 | Ticks 66 | Pos (150.3, 64.0, 200.5)
Frame 60 | FPS 359.7 | Accumulator 0.001 | Alpha 0.066 | Ticks 66 | Pos (150.3, 64.0, 200.5)
```

Notice: **Position identical regardless of FPS** ✓

---

## Spiral of Death Example

```cpp
// BEFORE: No clamping
// Lag spike for 500ms
dt = 0.5;
accumulator += 0.5;  // Now 0.517

int ticks_needed = 0.517 / 0.0151 = 34 ticks
// Processing 34 ticks takes ~1000ms
// Next frame: dt = 1.0 seconds
// Accumulator += 1.0
// Ticks needed = 66 + prev = 100 ticks
// Processing 100 ticks takes 2000ms
// → CRASH

// AFTER: With clamping
dt = 0.5;
if (dt > 0.25) dt = 0.25;  // ← CLAMP
accumulator += 0.25;  // Now 0.267

int ticks_needed = 0.267 / 0.0151 = 17 ticks
// Processing 17 ticks takes ~35ms
// Next frame: dt = ~0.1s (game recovering)
// accumulator += 0.1
// Ticks needed = 6 ticks + remaining = fine
// Game recovers gracefully
```

---

## Common Mistakes

| Mistake                             | Fix                                 |
| ----------------------------------- | ----------------------------------- |
| Using `frame_dt` in physics         | Use `TICK_DURATION`                 |
| Not clamping `frame_time`           | Add `if (dt > 0.25) dt = 0.25;`     |
| No interpolation                    | Calculate `alpha` and use in render |
| Processing input in render loop     | Sample in UpdateTick instead        |
| Tick rate changes mid-game          | Restart engine or reload            |
| Server/client mismatched tick rates | Use same TICK_DURATION              |
| Multiple ticks per frame causes lag | That's normal; should still be <1ms |
| Positions jump between ticks        | Enable interpolation in render      |

---

## Performance Notes

- 66 Hz = baseline CPU per tick
- 100 Hz = ~1.5x CPU
- 128 Hz = ~1.9x CPU
- Rendering at unlimited FPS = near free (interpolation is cheap)

---

## Relationship to Movement System

Your movement code (from movement_prompt.md) uses these constants:

```cpp
const float SPEED_RUN = 190.0f;      // HU/s
const float SV_ACCELERATE = 5.6f;    // HU/s²
```

In the tick system:

```cpp
// Each tick applies:
accel_per_tick = SV_ACCELERATE * TICK_DURATION * wish_speed
                = 5.6 * 0.0151 * 190
                = 16.1 HU per tick

// Per second:
16.1 * 66 = 1062.6 HU/s velocity gain

// With distance:
velocity_reached = 190 HU/s

// Time to reach max speed:
190 / 1062.6 = 0.179 seconds (12 ticks)
```

This stays consistent at any FPS now!
