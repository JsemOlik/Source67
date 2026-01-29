# Integration Guide: Movement + Tick System

This document shows how your movement system and tick system work together.

---

## The Complete Picture

```
60 FPS Monitor               Movement Constants           Tick System
│                            │                            │
Render Frame 1 (0.017s)      Velocity = 190 HU/s         Tick 0
│                            Accel = 5.6 HU/s²           ├─ Input: W pressed
├─ Accumulate 0.017s
├─ No tick yet
└─ Display with alpha=1.0

Render Frame 2 (0.017s)      Per-tick formulas:           Tick 1
│                            accel = 5.6 * 0.0151        ├─ Apply friction
├─ Accumulate total 0.034s   friction = 4.8 * 0.0151     ├─ Calculate wish_dir
├─ Tick 0 due! Execute       max_speed = 190             ├─ Accelerate: +2.5 HU/s
├─ Save prev_state           gravity = 800 * 0.0151      └─ Now at 2.5 HU/s
├─ UpdateTick(0.0151s)
├─ Remaining: 0.019s         All use TICK_DURATION       Tick 2
├─ alpha = 0.019/0.0151      not frame_dt!               ├─ Accelerate: +2.5 HU/s
└─ Display interpolated                                   └─ Now at 5.0 HU/s

Render Frame 3 (0.017s)
│
├─ Accumulate total 0.051s
├─ Tick 1 due! Execute
├─ Tick 2 due! Execute
├─ Remaining: 0.019s
└─ Display interpolated
```

---

## Code Structure

```cpp
class GameEngine {
    // ===== TICK SYSTEM =====
    const float TICK_DURATION = 1.0f / 66.0f;
    double accumulator = 0.0;
    GameState current_state, previous_state;

    // ===== MOVEMENT CONSTANTS =====
    // (From movement system)
    const float SPEED_RUN = 190.0f;
    const float SV_ACCELERATE = 5.6f;
    const float SV_AIRACCELERATE = 12.0f;
    const float SV_FRICTION = 4.8f;

    void MainLoop() {
        // Tick system main loop
        double dt = (now() - prev_time);
        if (dt > 0.25) dt = 0.25;

        accumulator += dt;

        // Process all due ticks
        while (accumulator >= TICK_DURATION) {
            previous_state = current_state;
            UpdateGameTick(current_state);  // ← Uses TICK_DURATION internally
            accumulator -= TICK_DURATION;
        }

        // Render with interpolation
        float alpha = accumulator / TICK_DURATION;
        RenderFrame(alpha);
    }

    void UpdateGameTick(GameState& state) {
        // This runs 66 times per second, always with same dt

        SampleInput(state);
        UpdateCrouch(state, TICK_DURATION);
        UpdateSprint(state, TICK_DURATION);
        ApplyMovement(state, TICK_DURATION);  // ← Movement system here
        CheckGround(state);
    }

    void ApplyMovement(GameState& state, float tick_dt) {
        // Movement system integration

        if (state.is_grounded) {
            // Friction applies: velocity *= (1 - 4.8 * 0.0151)
            ApplyFriction(state, tick_dt);

            // Ground acceleration applies
            GroundMove(state, tick_dt);
        } else {
            // Air acceleration applies: max 12 * 0.0151 = 0.181 HU per tick
            AirMove(state, tick_dt);

            // Gravity applies: velocity.y -= 800 * 0.0151 = 12.08 HU/s per tick
            state.velocity.y -= GRAVITY * tick_dt;
        }
    }
};
```

---

## Movement in Fixed Timestep

### Ground Acceleration Example

```
Input: W (forward)
wish_speed = 190 HU/s
sv_accelerate = 5.6 HU/s²
dt = TICK_DURATION = 0.0151s

Tick 0:
  currentspeed = dot(velocity[0,0,0], forward) = 0
  addspeed = 190 - 0 = 190
  accelspeed = 5.6 * 0.0151 * 190 = 16.1 HU
  velocity = [0, 0, 16.1]

Tick 1:
  currentspeed = dot([0,0,16.1], forward) = 16.1
  addspeed = 190 - 16.1 = 173.9
  accelspeed = 5.6 * 0.0151 * 190 = 16.1 HU
  velocity = [0, 0, 32.2]

Tick 2:
  ... accelspeed = 16.1 ...
  velocity = [0, 0, 48.3]

After ~12 ticks: velocity ≈ 190 HU/s (max speed reached)
```

**Key insight**: Because dt is always 0.0151s, every acceleration is identical. No matter if you render at 30 FPS or 300 FPS, after 12 ticks you've always gained the same velocity.

### Air Strafing Example

```
At start: velocity = [200, 0, 0] (moving right)
           wish_dir = [0, 0, 1] (forward, perpendicular to velocity)
           wish_speed = 30 (capped)

sv_airaccelerate = 12.0 HU/s²
dt = 0.0151s

Tick N:
  currentspeed = dot([200,0,0], [0,0,1]) = 0
  addspeed = 30 - 0 = 30 (MAXIMUM!)
  accelspeed = 12 * 0.0151 * 30 = 5.45 HU
  velocity = [200, 0, 5.45]

Tick N+1:
  currentspeed = dot([200,0,5.45], [0,0,1]) = 5.45
  addspeed = 30 - 5.45 = 24.55
  accelspeed = 12 * 0.0151 * 30 = 5.45 HU
  velocity = [200, 0, 10.9]

...continues accumulating in Z direction...

After ~6 ticks: velocity.z ≈ 30 HU/s
Speed increases from 200 to sqrt(200² + 30²) ≈ 202 HU/s per strafe direction change
```

---

## Framerate Independence Proof

### Test Case: Move Forward 5 Seconds

```
Scenario A: 60 FPS
├─ Frames = 300 per 5 seconds
├─ Ticks = 330 per 5 seconds (66 Hz * 5)
├─ Per tick acceleration = 16.1 HU
├─ Final velocity = 190 HU/s
└─ Distance = some value X

Scenario B: 300 FPS
├─ Frames = 1500 per 5 seconds
├─ Ticks = 330 per 5 seconds (same!)
├─ Per tick acceleration = 16.1 HU (same!)
├─ Final velocity = 190 HU/s (same!)
└─ Distance = EXACTLY value X

Scenario C: 1000 FPS
├─ Frames = 5000 per 5 seconds
├─ Ticks = 330 per 5 seconds (same!)
├─ Per tick acceleration = 16.1 HU (same!)
├─ Final velocity = 190 HU/s (same!)
└─ Distance = EXACTLY value X

PROOF: Same tick count = same acceleration = same distance
       Rendering FPS is irrelevant (interpolation handles it)
```

---

## Input Timing Diagram

```
30 FPS (33.33ms per frame)
Frame:  0        33       67       100      133      166
        │        │        │        │        │        │
        │    w:pressed   │        │        │   w:rel │
Tick:   0        0        1        1        2        2
        │        │        │        │        │        │
Input sampled at tick:
        Tick 0: W pressed during this tick
        Tick 1: W pressed during this tick
        Tick 2: W released during this tick

300 FPS (3.33ms per frame)
Frame:  0 3 6 9 12 15 18 21 24 27 30 33 36 39 42 ...
        │       │       │       │       │       │
        │ w:pressed          w:released
Tick:   0       0       1       1       2       2
        │       │       │       │       │       │
Input sampled at tick:
        Tick 0: W pressed (captured at frame 9 or so)
        Tick 1: W pressed (frame 30 or so)
        Tick 2: W released (frame 36 or so)

Result: Same ticks process same input!
        30 FPS and 300 FPS both sample W for ticks 0 and 1
        Both release W at tick 2
```

---

## Interpolation During Movement

```
Tick 0 (t=0.00s):      Tick 1 (t=0.015s):      Tick 2 (t=0.030s):
position = [0,0,0]     position = [0,0,2.85]   position = [0,0,5.70]
velocity = [0,0,0]     velocity = [0,0,190]    velocity = [0,0,190]

Rendering at 300 FPS (every 3.33ms):

t=0.000s:  alpha=0.00  pos=[0,0,0]         (at Tick 0)
t=0.003s:  alpha=0.20  pos=[0,0,0.57]      (20% toward Tick 1)
t=0.007s:  alpha=0.46  pos=[0,0,1.31]      (46% toward Tick 1)
t=0.010s:  alpha=0.66  pos=[0,0,1.88]      (66% toward Tick 1)
t=0.013s:  alpha=0.86  pos=[0,0,2.45]      (86% toward Tick 1)
t=0.015s:  alpha=0.00  pos=[0,0,2.85]      (at Tick 1)
t=0.018s:  alpha=0.20  pos=[0,0,3.42]      (20% toward Tick 2)
t=0.023s:  alpha=0.53  pos=[0,0,4.27]      (53% toward Tick 2)
t=0.028s:  alpha=0.86  pos=[0,0,5.12]      (86% toward Tick 2)
t=0.030s:  alpha=0.00  pos=[0,0,5.70]      (at Tick 2)

Result: SMOOTH position progression
        No jumping or stuttering between ticks
        Player movement appears continuous
```

---

## Debugging Your Combined System

```cpp
void PrintMovementStats() {
    static float print_timer = 0;
    print_timer += TICK_DURATION;

    if (print_timer >= 1.0f) {
        printf("=== Tick %d (%.2fs) ===\n",
            tick_count,
            tick_count * TICK_DURATION);

        printf("Position: (%.2f, %.2f, %.2f)\n",
            current_state.position.x,
            current_state.position.y,
            current_state.position.z);

        printf("Velocity: (%.2f, %.2f, %.2f) [magnitude: %.2f]\n",
            current_state.velocity.x,
            current_state.velocity.y,
            current_state.velocity.z,
            glm::length(current_state.velocity));

        printf("Grounded: %s, Sprinting: %s, Crouching: %s\n",
            current_state.is_grounded ? "yes" : "no",
            current_state.is_sprinting ? "yes" : "no",
            current_state.is_crouching ? "yes" : "no");

        print_timer = 0;
    }
}

void PrintRenderStats() {
    static float render_timer = 0;
    static int frame_count = 0;
    static double frame_time_sum = 0;

    frame_count++;
    render_timer += frame_time;
    frame_time_sum += frame_time;

    if (render_timer >= 1.0) {
        printf("=== Frame Stats (last second) ===\n");
        printf("Frames: %d\n", frame_count);
        printf("Average FPS: %.1f\n", frame_count);
        printf("Alpha range: 0.00 to 0.99 (should be full range)\n");
        printf("Accumulator: %.6f / %.6f\n",
            accumulator, TICK_DURATION);

        frame_count = 0;
        frame_time_sum = 0;
        render_timer = 0;
    }
}
```

---

## Order of Operations in UpdateGameTick

**CRITICAL: This exact order matters!**

```cpp
void UpdateGameTick(GameState& state, float tick_dt) {
    // TICK DURATION IS: tick_dt = 0.0151s ALWAYS

    // 1. SAMPLE INPUT
    //    Read current key states
    //    Convert to forward_input, side_input (-1 to 1)
    SampleInput(state);

    // 2. UPDATE VIEW ANGLES
    //    Rotate camera based on mouse input
    UpdateViewAngles(state);

    // 3. UPDATE CROUCH
    //    Transition between crouch heights
    UpdateCrouch(state, tick_dt);

    // 4. UPDATE SPRINT
    //    Track sprint duration and recovery
    UpdateSprint(state, tick_dt);

    // 5. APPLY MOVEMENT
    //    This is where your movement system runs!
    //    If grounded: Friction + Ground Acceleration
    //    If airborne: Air Acceleration + Gravity
    ApplyMovement(state, tick_dt);

    // 6. CHECK COLLISION & GROUND
    //    Move player in world
    //    Detect if feet touching ground
    MoveAndCollide(state);  // Uses state.position and state.velocity

    // 7. HANDLE JUMP
    //    If spacebar pressed AND grounded, jump
    //    Sets state.velocity.y = 268
    //    Clears jump_pressed flag
    HandleJump(state);
}
```

**Why this order?**

- Input must come first (affects movement)
- Friction must come before acceleration (affects how much we can accelerate)
- Collision must come before jump check (need to know if grounded)
- Jump check must come last (jump uses collision results)

---

## Performance Expectations

With this combined system:

```
60 FPS rendering, 66 Hz tick:
├─ ~4 render frames per tick
├─ Accumulator processes 1 tick per frame usually
├─ Movement is smooth and responsive
└─ CPU cost: minimal (16ms per frame, physics is fast)

300 FPS rendering, 66 Hz tick:
├─ ~4.5 render frames per tick
├─ Interpolation handles smooth motion at any FPS
├─ Same physics as 60 FPS (same ticks)
├─ CPU cost: GPU-bound, physics unchanged
└─ Movement is smooth and responsive

30 FPS rendering, 66 Hz tick:
├─ ~0.45 render frames per tick
├─ Some ticks will run without render in between
├─ Accumulator will have leftover time
├─ Interpolation between distant tick states
└─ May look choppy but movement is correct

Lag spike recovery:
├─ Frame takes 500ms
├─ Clamped to 250ms
├─ 16 ticks processed instead of 33
├─ System recovers smoothly
└─ No "spiral of death"
```

---

## Common Integration Mistakes

| Mistake                            | Impact                           | Fix                                         |
| ---------------------------------- | -------------------------------- | ------------------------------------------- |
| Using `frame_dt` in movement       | 30 FPS moves slower than 300 FPS | Use `tick_dt = TICK_DURATION`               |
| Not clamping `frame_time`          | Lag spikes crash game            | Add `if (dt > 0.25) dt = 0.25;`             |
| Skipping interpolation             | Stuttery movement                | Store prev_state and lerp in render         |
| Processing input in render         | Input might miss a tick          | Sample input in UpdateGameTick              |
| Mismatched tick rates              | Physics diverges between clients | Use same TICK_DURATION everywhere           |
| Forgetting to save `prev_state`    | Interpolation breaks             | `prev_state = current_state;` before update |
| Accumulating floating point errors | Drift over time                  | Use double for accumulator                  |

---

## Validation Checklist

- [ ] Movement constants (SPEED*\*, SV*\*) defined
- [ ] Tick constants (TICK_RATE, TICK_DURATION) defined
- [ ] Main loop implements accumulator pattern
- [ ] Frame time clamped to 0.25s
- [ ] Previous state saved before each tick
- [ ] UpdateGameTick called with TICK_DURATION
- [ ] Movement code uses only TICK_DURATION, not frame dt
- [ ] Interpolation alpha calculated and used
- [ ] Same movement at 30, 60, and 300 FPS
- [ ] No lag spike causes spiral of death
- [ ] No stutter in rendering
- [ ] Input sampled once per tick
- [ ] Jump works consistently at any FPS
- [ ] Air strafing works consistently at any FPS
- [ ] Crouch animation smooth

All 15 items checked? ✓ You're good to go!
