# Tick System Visual Reference

Quick visual guides for understanding how the tick system works.

---

## Main Loop Flowchart

```
┌─────────────────────────────────────┐
│      Game Engine Start              │
│  accumulator = 0.0                  │
│  prev_time = GetTime()              │
└──────────────┬──────────────────────┘
               │
               ▼
        ┌──────────────┐
        │  Frame Loop  │
        │  (Every FPS) │
        └──────┬───────┘
               │
               ▼
    ┌─────────────────────────┐
    │ curr_time = GetTime()   │
    │ dt = curr_time - prev   │
    │ prev_time = curr_time   │
    └─────────────┬───────────┘
                  │
                  ▼
        ┌──────────────────┐
        │ if dt > 0.25:    │
        │   dt = 0.25      │◄─── Spiral of Death Prevention!
        └────────┬─────────┘
                 │
                 ▼
        ┌───────────────────┐
        │ accumulator += dt │
        └────────┬──────────┘
                 │
                 ▼
    ┌────────────────────────────┐
    │ while (acc >= TICK_DUR):   │
    │   prev_state = curr_state  │
    │   UpdateTick(curr_state)   │
    │   acc -= TICK_DURATION     │
    └────────┬───────────────────┘
             │
             ▼
    ┌──────────────────────────┐
    │ alpha = acc/TICK_DUR     │
    │ RenderFrame(alpha)       │
    └──────────┬───────────────┘
               │
               ▼ (repeat loop)
```

---

## Accumulator Over Time

```
Frame 0 (t=0.000s, dt=0.010s):
  accumulator = 0.010
  0.010 < 0.0151? YES → No tick yet
  Render with alpha = 0.010/0.0151 = 0.66

Frame 1 (t=0.010s, dt=0.008s):
  accumulator = 0.010 + 0.008 = 0.018
  0.018 >= 0.0151? YES → Execute Tick 0
  accumulator = 0.018 - 0.0151 = 0.0029
  0.0029 < 0.0151? YES → Stop
  Render with alpha = 0.0029/0.0151 = 0.19

Frame 2 (t=0.018s, dt=0.020s):
  accumulator = 0.0029 + 0.020 = 0.0229
  0.0229 >= 0.0151? YES → Execute Tick 1
  accumulator = 0.0229 - 0.0151 = 0.0078
  0.0078 < 0.0151? YES → Stop
  Render with alpha = 0.0078/0.0151 = 0.52

Frame 3 (t=0.038s, dt=0.022s):
  accumulator = 0.0078 + 0.022 = 0.0298
  0.0298 >= 0.0151? YES → Execute Tick 2
  accumulator = 0.0298 - 0.0151 = 0.0147
  0.0147 < 0.0151? YES → Stop
  Render with alpha = 0.0147/0.0151 = 0.97

Frame 4 (t=0.060s, dt=0.009s):
  accumulator = 0.0147 + 0.009 = 0.0237
  0.0237 >= 0.0151? YES → Execute Tick 3
  accumulator = 0.0237 - 0.0151 = 0.0086
  0.0086 < 0.0151? YES → Stop
  Render with alpha = 0.0086/0.0151 = 0.57
```

Note: Ticks occur every 15.15ms regardless of frame timing!

---

## Tick and Frame Relationship

```
Perfect sync at 66 FPS (15.15ms per frame):
Frames:  |---Frame---|---Frame---|---Frame---|
Ticks:   |---Tick---|---Tick---|---Tick---|
         Perfect alignment

Misalignment at 60 FPS (16.67ms per frame):
Frames:  |--Frame--|--Frame--|--Frame--|
Ticks:   |--T--|--T--|--T--|--T--|
         Ticks finish first, then frame, then accumulator builds for next tick

Variable FPS with vsync off:
Frames:  |--F--|---Frame---|F|-F|----Frame----|
Ticks:   |--T--|--T--|--T--|--T--|--T--|--T--|
         Ticks stay perfectly spaced, frames vary
         Accumulator compensates for frame variations
```

---

## Movement Acceleration Timeline

```
Input: Hold W (forward), no air strafing

Tick 0 (t=0.000s):
  velocity = [0, 0, 0] HU/s
  Input: forward = 1.0
  Acceleration = 5.6 * 0.0151 * 190 = 16.1 HU
  velocity = [0, 0, 16.1] HU/s
  distance = 0 HU

Tick 1 (t=0.0151s):
  velocity = [0, 0, 16.1] HU/s
  Acceleration = 16.1 HU (same every tick!)
  velocity = [0, 0, 32.2] HU/s
  distance = 2.85 HU (moved 2.85 HU this tick)

Tick 2 (t=0.0303s):
  velocity = [0, 0, 32.2] HU/s
  Acceleration = 16.1 HU
  velocity = [0, 0, 48.3] HU/s
  distance = 5.70 HU total

... 10 more ticks of identical 16.1 HU acceleration ...

Tick 12 (t=0.1818s):
  velocity ≈ [0, 0, 190] HU/s (max speed reached!)
  Acceleration = 0 HU (can't go faster)
  velocity = [0, 0, 190] HU/s (stays constant)

Tick 13+ (t=0.1969s+):
  velocity = [0, 0, 190] HU/s (constant)
  acceleration = 0
  Moving at constant 190 HU/s forever

Total time to reach max speed: ~12 ticks = 0.18 seconds

This is IDENTICAL whether you render at 30 FPS or 300 FPS!
```

---

## Interpolation Visualization

```
Tick State:        Tick 0              Tick 1              Tick 2
Position:          pos = 0             pos = 2.85          pos = 5.70
Time:              t = 0.000           t = 0.0151          t = 0.0303

Rendering at 120 FPS (8.33ms per frame, 4.5 frames per tick):

Frame 0 (t=0.0000s): alpha = 0.00 ──────▲────── pos = 0.00
Frame 1 (t=0.0083s): alpha = 0.55 ──────┐──▲─── pos = 1.57
Frame 2 (t=0.0167s): alpha = 1.10 [wrap] ▼┐──▲─ pos = 3.14 (past Tick 1!)
Frame 3 (t=0.0250s): alpha = 0.65 ──────┐──▲─── pos = 4.19
Frame 4 (t=0.0333s): alpha = 0.20 ──────┐──▼─── pos = 5.13
Frame 5 (t=0.0417s): alpha = 0.75 ──────┐──▲─── pos = 5.45

When alpha > 1.0: Frame came after tick, use interpolation to next tick

Visual result: Smooth continuous motion from 0 to 5.70 over time
```

---

## Air Strafing Per-Tick

```
Initial state:
  velocity = [100, 0, 0] HU/s (moving right)
  wish_dir = [0, 0, 1] (forward, perpendicular!)
  wish_speed = 30 (capped in air)

Tick N:
  currentspeed = dot([100,0,0], [0,0,1]) = 0  ◄── KEY: perpendicular = 0!
  addspeed = 30 - 0 = 30  ◄── MAXIMUM!
  accelspeed = 12 * 0.0151 * 30 = 5.45 HU
  velocity = [100, 0, 5.45]

Tick N+1:
  currentspeed = dot([100,0,5.45], [0,0,1]) = 5.45
  addspeed = 30 - 5.45 = 24.55
  accelspeed = 12 * 0.0151 * 30 = 5.45 HU
  velocity = [100, 0, 10.9]

Tick N+2:
  currentspeed = 10.9
  addspeed = 19.1
  accelspeed = 5.45 HU
  velocity = [100, 0, 16.35]

...continues...

Tick N+6:
  velocity = [100, 0, 32.7] HU/s
  speed = sqrt(100² + 32.7²) = 104.9 HU/s  ◄── Gained 4.9 HU/s per tick!

This is why air strafing works: perpendicular movement allows constant
maximum acceleration. Same acceleration every tick = predictable speed gain.
```

---

## Framerate Independence Proof

```
Test: Move forward for exactly 66 ticks (1 second at tick rate)

At 30 FPS (33ms frames):
├─ Frames per second: 30
├─ Ticks per second: 66 (fixed!)
├─ Ticks in 1 second: 66
├─ Acceleration per tick: 16.1 HU
├─ Final velocity: ~190 HU/s (max speed)
└─ Distance traveled: ~47.5 HU

At 60 FPS (16.7ms frames):
├─ Frames per second: 60
├─ Ticks per second: 66 (fixed!)
├─ Ticks in 1 second: 66 (SAME!)
├─ Acceleration per tick: 16.1 HU (SAME!)
├─ Final velocity: ~190 HU/s (SAME!)
└─ Distance traveled: ~47.5 HU (SAME!)

At 300 FPS (3.3ms frames):
├─ Frames per second: 300
├─ Ticks per second: 66 (FIXED!)
├─ Ticks in 1 second: 66 (EXACTLY SAME!)
├─ Acceleration per tick: 16.1 HU (EXACTLY SAME!)
├─ Final velocity: ~190 HU/s (EXACTLY SAME!)
└─ Distance traveled: ~47.5 HU (EXACTLY SAME!)

PROOF: Tick count is identical regardless of frame rate!
```

---

## Input Sampling Diagram

```
Logical time: |---Tick 0---|---Tick 1---|---Tick 2---|---Tick 3---|
Real time:    0ms         15ms         30ms         45ms         60ms

60 FPS case (16.7ms per frame):
Frames:      0     16.7    33.3    50    66.7
             |      |       |       |      |
Keys:    W[pressed]        W[released]
             |______Y______|______N______|

Tick sampling:
  Tick 0 (t=0ms):   Samples keyboard → W=pressed → forward=1
  Tick 1 (t=15ms):  Samples keyboard → W=pressed → forward=1
  Tick 2 (t=30ms):  Samples keyboard → W=released → forward=0
  Tick 3 (t=45ms):  Samples keyboard → W=released → forward=0

Result: Forward applied for ticks 0 and 1, released at tick 2

300 FPS case (3.3ms per frame):
Frames:      0 3 6 9 12 15 18 21 24 27 30 33 36 39 42 45 48 51 54
             |      |       |       |       |       |       |      |
Keys:    W[pressed]                   W[released]
             |_______________________|__________________|

Tick sampling:
  Tick 0 (t=0ms):   Samples at frame 0 → W=pressed → forward=1
  Tick 1 (t=15ms):  Samples at frame 45 → W=pressed → forward=1
  Tick 2 (t=30ms):  Samples at frame 90 → W=released → forward=0
  Tick 3 (t=45ms):  Samples at frame 135 → W=released → forward=0

Result: IDENTICAL! Forward applied for ticks 0 and 1, released at tick 2

KEY INSIGHT: Tick sampling is deterministic! Same key press will be
sampled during the same tick regardless of how many frames per tick!
```

---

## Spiral of Death Prevention

```
Without clamping frame_time:

t=0.000s:  dt=0.016 → accumulator=0.016 → 1 tick
t=0.016s:  dt=0.017 → accumulator=0.017 → 1 tick → total 2 ticks/33ms ✓

Lag spike! (GPU stalls for 500ms):

t=0.516s:  dt=0.500 → accumulator=0.500
           Ticks needed = 0.500 / 0.0151 = 33 ticks!
           Processing 33 ticks at 1 tick every 1.5ms = 49ms to process
           But we're also rendering, so actually 100ms+ to process

t=0.616s:  dt=0.100 (recovery) → accumulator=0.100 - 0.015*33 + 0.100
           Still have leftover, need to process more
           But CPU still catching up from previous lag

t=0.716s:  dt=0.150 → More lag ticks pile up
           System never catches up!

Result: ❌ CRASH (spiral of death)

---

With clamping frame_time:

t=0.516s:  dt=0.500 → clamped to 0.250 → accumulator=0.250
           Ticks needed = 0.250 / 0.0151 = 16 ticks
           Processing 16 ticks = ~25ms (fast!)

t=0.541s:  dt=0.100 (recovery) → accumulator=(0.250-16*0.0151)+0.100
           accumulator=0.1035 → 0 ticks due
           Render at normal speed

t=0.557s:  dt=0.020 → accumulator=0.1235 → 0 ticks yet
t=0.577s:  dt=0.020 → accumulator=0.1435 → 0 ticks yet
t=0.597s:  dt=0.020 → accumulator=0.1635 → 1 tick due!
           accumulator=0.148 → continue normally

Result: ✓ RECOVERY (game stays responsive)
```

---

## State Interpolation

```
previous_state:  │position=[0,0,0]
                 │velocity=[0,0,190]
                 │grounded=true

current_state:   │position=[0,0,2.85]
                 │velocity=[0,0,190]
                 │grounded=true

alpha values for rendering:

alpha=0.0:  display_position = [0,0,0]     + [0,0,0]*0.0 = [0,0,0]
alpha=0.1:  display_position = [0,0,0]     + [0,0,2.85]*0.1 = [0,0,0.285]
alpha=0.2:  display_position = [0,0,0]     + [0,0,2.85]*0.2 = [0,0,0.570]
alpha=0.3:  display_position = [0,0,0]     + [0,0,2.85]*0.3 = [0,0,0.855]
alpha=0.4:  display_position = [0,0,0]     + [0,0,2.85]*0.4 = [0,0,1.140]
alpha=0.5:  display_position = [0,0,0]     + [0,0,2.85]*0.5 = [0,0,1.425]
alpha=0.6:  display_position = [0,0,0]     + [0,0,2.85]*0.6 = [0,0,1.710]
alpha=0.7:  display_position = [0,0,0]     + [0,0,2.85]*0.7 = [0,0,1.995]
alpha=0.8:  display_position = [0,0,0]     + [0,0,2.85]*0.8 = [0,0,2.280]
alpha=0.9:  display_position = [0,0,0]     + [0,0,2.85]*0.9 = [0,0,2.565]
alpha=0.99: display_position = [0,0,0]     + [0,0,2.85]*0.99= [0,0,2.822]

Result: Smooth progression from [0,0,0] to [0,0,2.85] over 15.15ms!
```

---

## Tick Count per FPS

```
Time elapsed:    1 second

At 30 FPS:       30 frames rendered
                 66 ticks executed (fixed!)
                 Alpha varies 0.0 to 0.999
                 Motion smooth (interpolated)

At 60 FPS:       60 frames rendered
                 66 ticks executed (same!)
                 Alpha varies 0.0 to 0.999
                 Motion smooth (interpolated)
                 More frames = smoother due to more interpolation points

At 300 FPS:      300 frames rendered
                 66 ticks executed (always same!)
                 Alpha varies 0.0 to 0.999
                 Motion very smooth (many interpolation points)
                 Higher FPS = subjectively "smoother" due to rendering

At 1000 FPS:     1000 frames rendered
                 66 ticks executed (unchanged!)
                 Alpha varies 0.0 to 0.999
                 Motion extremely smooth
                 But no physics benefit - physics is always 66 ticks/sec
```

The key insight: **Rendering FPS affects visual smoothness through interpolation.
Physics ticks are always constant. Both working together = perfect motion!**

---

## Tick Duration Reference

```
TICK_DURATION = 1.0f / TICK_RATE

Common rates:
  30 Hz:  1.0 / 30 = 0.0333 seconds (33.3 ms)
  60 Hz:  1.0 / 60 = 0.0167 seconds (16.7 ms)
  64 Hz:  1.0 / 64 = 0.0156 seconds (15.6 ms)
  66 Hz:  1.0 / 66 = 0.0152 seconds (15.2 ms)  ◄── Source Engine standard
  100 Hz: 1.0 / 100 = 0.0100 seconds (10.0 ms)
  128 Hz: 1.0 / 128 = 0.0078 seconds (7.8 ms)

Lower Hz = longer per-tick (more time per physics step, faster computation)
Higher Hz = shorter per-tick (more steps needed, more accurate)

Source chose 66 Hz as good balance:
  - 4-5 ticks per 60 Hz frame (reasonable)
  - Fast enough for responsive gameplay
  - Slow enough for low CPU cost
  - Historical: matches Quake tradition
```

---

That's the visual reference! Use these diagrams to understand the tick system
at a glance without reading all 474 lines of research!
