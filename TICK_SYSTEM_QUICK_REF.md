# Source67 Tick System - Quick Developer Reference

## Constants (in Application.h)

```cpp
static constexpr float TICK_RATE = 66.0f;              // Hz (ticks per second)
static constexpr float TICK_DURATION = 1.0f / 66.0f;   // ~0.015151515f seconds
static constexpr float MAX_FRAME_TIME = 0.25f;         // Max 250ms per frame
```

## Main Loop Flow (Application::Run)

```
Every Frame:
1. Measure time since last frame (with double precision)
2. Clamp frame_time to prevent spiral of death
3. Add frame_time to accumulator
4. While accumulator has enough time for a tick:
   - Save previous_state = current_state
   - Call UpdateGameTick(TICK_DURATION)  ← Always 0.015151515s
   - Subtract TICK_DURATION from accumulator
5. Calculate alpha = accumulator / TICK_DURATION
6. Call RenderFrame(alpha) with interpolation
7. Swap buffers and poll events
```

## Physics Tick (Application::UpdateGameTick)

**Frequency**: Exactly 66 times per second (66 Hz)
**Delta Time**: Always `TICK_DURATION` (0.015151515 seconds)

```cpp
void Application::UpdateGameTick(float tick_dt) {
  // Only runs during Play mode
  if (m_SceneState != SceneState::Play) return;
  
  // 1. Update player with FIXED timestep
  m_PlayerController->OnUpdate(Timestep(tick_dt));
  
  // 2. Update physics with FIXED timestep
  PhysicsSystem::OnUpdate(Timestep(tick_dt));
  
  // 3. Store state for interpolation
  m_CurrentState.player_position = ...;
  m_CurrentState.yaw = ...;
  m_CurrentState.pitch = ...;
}
```

## Rendering (Application::RenderFrame)

**Parameter**: `float alpha` - Interpolation factor (0.0 to ~0.999)

```cpp
void Application::RenderFrame(float alpha) {
  // Interpolate between previous and current physics states
  
  glm::vec3 interpolated_pos = glm::mix(
      m_PreviousState.player_position,
      m_CurrentState.player_position,
      alpha
  );
  
  float interpolated_yaw = glm::mix(
      m_PreviousState.yaw,
      m_CurrentState.yaw,
      alpha
  );
  
  // Apply to camera for smooth rendering
  m_Camera->SetPosition(interpolated_pos + glm::vec3(0, 1.7, 0));
  m_Camera->SetYaw(interpolated_yaw);
  
  // Render scene...
}
```

## GameState Structure

```cpp
struct GameState {
    glm::vec3 player_position;
    glm::vec3 player_velocity;
    float yaw;              // Camera horizontal rotation
    float pitch;            // Camera vertical rotation
    
    // Movement state
    bool is_grounded;
    bool is_sprinting;
    float sprint_remaining;
    float sprint_recovery_time;
    bool is_crouching;
    float crouch_transition;
    
    // Input (sampled during tick)
    float forward_input;    // -1, 0, 1
    float side_input;       // -1, 0, 1
    bool jump_pressed;
    bool sprint_pressed;
    bool crouch_pressed;
    
    float eye_height;       // For camera positioning
};
```

## Key Principles

### ✅ DO

- **Use TICK_DURATION for all physics calculations**
- **Interpolate all positions/rotations during rendering**
- **Store both current and previous states**
- **Clamp frame_time to prevent spiral of death**

### ❌ DON'T

- **Never use variable frame delta time in physics**
- **Don't skip interpolation** (causes stuttering)
- **Don't forget to save previous_state before tick**
- **Don't remove frame_time clamping**

## Example: Adding New Interpolated State

1. Add field to `GameState`:
   ```cpp
   struct GameState {
       // ... existing fields ...
       float new_value;
   };
   ```

2. Update in `UpdateGameTick()`:
   ```cpp
   m_CurrentState.new_value = CalculateNewValue();
   ```

3. Interpolate in `RenderFrame()`:
   ```cpp
   float interpolated_value = glm::mix(
       m_PreviousState.new_value,
       m_CurrentState.new_value,
       alpha
   );
   ```

## Debugging

### Check Tick Rate
```cpp
// In UpdateGameTick()
static uint64_t tick_start_time = m_TickNumber;
static double time_start = glfwGetTime();

if (glfwGetTime() - time_start >= 10.0) {
    uint64_t ticks = m_TickNumber - tick_start_time;
    S67_CORE_INFO("Ticks in 10 seconds: {0} (expected 660)", ticks);
    // Should be exactly 660 (66 Hz * 10 seconds)
    tick_start_time = m_TickNumber;
    time_start = glfwGetTime();
}
```

### Check Alpha Range
```cpp
// In RenderFrame()
static float min_alpha = 1.0f, max_alpha = 0.0f;
min_alpha = std::min(min_alpha, alpha);
max_alpha = std::max(max_alpha, alpha);

// Every second, print range (should be 0.0 to ~0.999)
S67_CORE_INFO("Alpha range: {0} to {1}", min_alpha, max_alpha);
```

### Verify Framerate Independence
```cpp
// Record position at start
static glm::vec3 start_pos = m_CurrentState.player_position;
static double start_time = glfwGetTime();

// After 10 seconds, check distance traveled
if (glfwGetTime() - start_time >= 10.0) {
    float distance = glm::distance(start_pos, m_CurrentState.player_position);
    S67_CORE_INFO("Distance traveled in 10s: {0}", distance);
    // Should be identical at 30 FPS, 60 FPS, and 300 FPS!
}
```

## Performance Metrics

| Render FPS | Ticks/Second | Ticks/Frame | Alpha Updates/Second |
|------------|--------------|-------------|----------------------|
| 30 FPS     | 66 Hz        | ~2.2 ticks  | 30 times             |
| 60 FPS     | 66 Hz        | ~1.1 ticks  | 60 times             |
| 144 FPS    | 66 Hz        | ~0.46 ticks | 144 times            |
| 300 FPS    | 66 Hz        | ~0.22 ticks | 300 times            |

**Note**: Physics always runs at 66 Hz regardless of render FPS!

## Common Issues

### "Game feels stuttery at high FPS"
- Check that interpolation is enabled in RenderFrame()
- Verify alpha is being calculated correctly
- Make sure previous_state is saved before each tick

### "Physics behaves differently at different framerates"
- Check that UpdateGameTick uses TICK_DURATION, not frame delta
- Verify no variable timestep in movement calculations
- Ensure PhysicsSystem uses fixed 66Hz timestep

### "Game crashes during lag spike"
- Verify frame_time is clamped to MAX_FRAME_TIME (0.25s)
- Check that accumulator doesn't run infinite ticks
- Add tick_count limit in while loop if needed

### "Interpolation looks wrong"
- Check that previous_state is updated BEFORE UpdateGameTick()
- Verify alpha ranges from 0.0 to ~0.999
- Make sure interpolation uses correct states

## Further Reading

- Primary Specification: `/source_engine/tick_system/tick_system_prompt.md`
- Integration Guide: `/source_engine/tick_system/intergration_guide.md`
- Implementation Summary: `/TICK_SYSTEM_IMPLEMENTATION.md`
- Glenn Fiedler's "Fix Your Timestep" article
