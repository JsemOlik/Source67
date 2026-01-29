# Source Engine Movement System Research

## Overview

The Source Engine movement system is derived from Quake 1/2 engine, featuring air strafing that originated as a programming quirk but became a core mechanic. The system has both grounded and aerial movement with distinct acceleration patterns.

## Core Constants & Values

### Unit System

- **Hammer Units (HU)**: 1 HU ≈ 0.025 meters (or 39.97 HU = 1 meter)
- All calculations use hammer units internally

### Ground Movement Speeds

- **Crouching**: 63.3 HU/s (walking while crouched, no sprint)
- **Walking**: 150 HU/s (alt+move, slower controlled movement)
- **Running**: 190 HU/s (normal forward movement)
- **Sprinting**: 320 HU/s (hold shift, 8-second duration, 2560 HU max distance)

### Acceleration Constants (Default Half-Life 2 / CS:GO)

- **sv_accelerate**: 5.6 (ground movement acceleration)
- **sv_airaccelerate**: 12 (air movement acceleration, increased from 6-9 in updates)
- **sv_friction**: 4.8 (ground deceleration/drag)

### Air Strafing Specific Values

- **maxAirWishSpeed**: 30 HU/s (cap on wish direction speed in air)
- **maxAirSpeedCap**: 30 HU/s (limits addspeed before reduction)
- Allows unlimited velocity accumulation in practice

### Player Dimensions

- **Standing Eye Level**: 64 units
- **Crouch Eye Level**: 28 units
- **Jump Height**: +21 units (total reach: 85 standing, 49 crouching)
- **Max Step Height**: 16 units
- **Collision Hull Width**: Based on bounding box

## Movement Mechanics

### 1. Ground Movement (Walking/Running)

**Process:**

1. Calculate wish direction from input (WASD keys)
2. Multiply by respective speeds (cl_forwardspeed, cl_sidespeed = 450 HU default)
3. Apply friction to decelerate when no input
4. Apply acceleration to approach max speed
5. Cap velocity at MaxSpeed (190 or 320 if sprinting)

**Friction Calculation:**

- Friction only applies on ground
- Removes velocity proportional to surface friction multiplier (1.0 default)
- Formula: `velocity *= (1 - friction * frametime)`
- Comes BEFORE acceleration in tick order

**Acceleration on Ground:**

```
currentspeed = dot(velocity, wishdir)
addspeed = MaxSpeed - currentspeed
if (addspeed <= 0) return
accelspeed = sv_accelerate * frametime * wishspeed * surface_friction
accelspeed = min(accelspeed, addspeed)  // Cap acceleration this frame
velocity += accelspeed * wishdir
```

### 2. Air Movement (Basic)

Without strafing, air movement is very limited:

- Acceleration capped at 30 HU/s (sv_airaccelerate applied to this cap)
- Forward/back movement affects Z direction
- Side movement affects X direction
- No friction in air
- Gravity constantly applied downward

### 3. Air Strafing (The Key Mechanic)

**How It Works:**

1. Player jumps (loses ground contact)
2. Holds strafe key (A or D)
3. Turns camera in strafe direction (or perpendicular to current velocity)
4. Due to dot product calculation, perpendicular input = currentspeed near 0
5. This makes addspeed = maxAirWishSpeed = 30 (maximum)
6. Acceleration applied in strafed direction, gaining speed

**Air Acceleration Function:**

```
void AirAccelerate(Vector3 wishdir, float wishspeed, float airaccelerate)
{
    // Clamp wish speed to air maximum
    if (wishspeed > maxAirWishSpeed)
        wishspeed = maxAirWishSpeed;  // 30 HU/s

    // Project current velocity onto wish direction
    float currentspeed = dot(velocity, wishdir);

    // Calculate how much we can accelerate
    float addspeed = wishspeed - currentspeed;

    // If already moving in wish direction at max, don't accelerate
    if (addspeed <= 0)
        return;

    // Calculate acceleration for this frame
    float accelspeed = airaccelerate * frametime * wishspeed;

    // Cap acceleration to not exceed addspeed
    if (accelspeed > addspeed)
        accelspeed = addspeed;

    // Apply acceleration
    velocity += accelspeed * wishdir;
}
```

**Why It Works - The Math:**

- When perpendicular (90°) to velocity: `dot(velocity, wishdir) ≈ 0`
- Therefore: `addspeed = 30 - 0 = 30` (maximum)
- Acceleration = `12 * frametime * 30` = significant gain
- When aligned forward: `dot(velocity, wishdir) ≈ velocity.magnitude`
- Therefore: `addspeed = 30 - 500 = -470` (negative, no acceleration)
- Optimal angle is slightly off perpendicular due to framerate dependency

### 4. Optimal Air Strafing Angle

- **Pure Quake I/Source**: 90° to velocity vector (perpendicular)
- **Frame-rate dependent**: Optimal angle changes with tickrate
- **General rule**: Turn camera to ~80-90° from current velocity direction
- **Speed gain**: Each frame can add 0-30 HU/s depending on angle perfection

### 5. Crouch Mechanics

- **Crouch Speed**: 63.3 HU/s max (much slower)
- **Crouch Height**: 28 units (vs 64 standing)
- **Usage**:
  - Combined with run+crouch = accelerated backhopping (ABH)
  - Can strafe-jump while crouching for speed boost
  - Transitions involve animation duration (affects jumpoff timing)

### 6. Jump Mechanics

- **Jump Velocity**: 268 HU/s upward (instant on press)
- **Momentum Preservation**: Forward/side momentum carries to air
- **Speed Boost Versions**:
  - **Circle Strafe Jump**: Sprint + strafe + jump while turning = higher initial speed
  - **Crouch Jump**: Duck before jumping = tighter control, can clear higher obstacles
  - **Bunny Hop**: Jump repeatedly without landing = continuous acceleration

## Key Advanced Techniques

### 1. Accelerated Back Hopping (ABH)

- Jump while facing backward
- Strafe while mid-air
- Land and immediately jump again (frame-perfect)
- Backwards velocity is treated as negative, causing acceleration to apply in opposite direction
- Can reach 500+ HU/s with perfect execution

### 2. Circle Strafing

- Hold sprint, strafe left/right, turn camera same direction
- Accelerates while building up speed before jumping
- Jump at peak speed for maximum initial velocity

### 3. Forward Strafing (AFH)

- Less common, requires precise S (back) tap timing on landing

### 4. Strafe Lurch (Titanfall-style, not in HL2)

- Immediate velocity boost on strafe input
- Source uses smooth acceleration instead

## Ground vs Air Comparison

| Property               | Ground                  | Air                           |
| ---------------------- | ----------------------- | ----------------------------- |
| Max Speed              | 190 HU/s (320 sprint)   | Unlimited (soft cap via wish) |
| Acceleration           | sv_accelerate (5.6)     | sv_airaccelerate (12)         |
| Friction               | Applied (4.8)           | None                          |
| Velocity Cap           | Hard limit              | Soft via addspeed cap (30)    |
| Movement Feel          | Responsive, decelerates | Floaty, inertial              |
| Acceleration Direction | Any                     | Perpendicular for strafe gain |

## Velocity Drag/Friction Details

### Ground Friction

- **Formula**: `velocity *= (1.0 - friction * frametime)`
- **Effective**: Exponential decay, not linear
- **Multiplier**: surface_friction (1.0 default, varies by surface)
- **Applied**: Before acceleration each tick

### Air Friction

- **None applied to player**
- **Gravity**: 32 ft/s² ≈ 800 HU/s² (constant downward acceleration)

### Velocity Exceeding Cap

- **On ground**: Friction scales up with excess velocity
  - At 2x MaxSpeed: Heavy braking
  - Intent: Push player back to speed cap naturally
- **In air**: No penalty, keeps velocity (enables strafing)

## Movement Tick Order

1. **Apply Friction** (ground only)
2. **Read Input** (WASD keys, camera look)
3. **Calculate Wish Direction** (from input + camera angle)
4. **Apply Acceleration** (ground or air based on grounded status)
5. **Apply Gravity** (constant downward)
6. **Collision & Movement** (move player, detect ground contact)
7. **Check Jump Input** (jump if grounded + jump pressed)

## Special Cases

### Surface Friction

- Wood/metal: 1.0 (default)
- Slippery surfaces: 0.5-0.8
- Affects acceleration and friction calculations

### Ramp Sliding

- Moving down slope while air-strafing
- Can maintain contact with surface while sliding
- Allows continued acceleration

### Sprint Duration

- 8 seconds max duration
- 8 seconds recovery before can sprint again
- Can bypass with crouch (slower speed but no sprint limit)

## Implementation Notes

1. **Floating Point Precision**: Use float for velocities, frame-perfect timing difficult
2. **Framerate Dependency**: Tickrate affects optimal strafe angle
3. **Input Buffering**: Source reads discrete key presses, not analog input
4. **Collision Detection**: Must check ground status each frame
5. **Velocity Clamping**: Never clamp velocity directly in air (breaks strafing)

## Source Material References

- Quake 1 GPL source: SV_AirAccelerate function
- Project Borealis: Unreal Engine implementation with HL2 parity
- Half-Life Physics Reference (jwchong.com)
- Speedrun Community: Practical testing with DeSinc, spikehunter, etc.
