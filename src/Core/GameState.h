#pragma once

#include <glm/glm.hpp>

namespace S67 {

/**
 * @brief GameState holds all physics state for tick system interpolation
 * 
 * The tick system maintains two GameState instances (current and previous)
 * to enable smooth rendering interpolation between fixed physics ticks.
 */
struct GameState {
    // Position and rotation
    glm::vec3 player_position = {0.0f, 2.0f, 0.0f};
    glm::vec3 player_velocity = {0.0f, 0.0f, 0.0f};
    float yaw = -90.0f;      // Horizontal rotation
    float pitch = 0.0f;      // Vertical rotation

    // Movement state
    bool is_grounded = false;
    bool is_sprinting = false;
    float sprint_remaining = 8.0f;
    float sprint_recovery_time = 0.0f;
    bool is_crouching = false;
    float crouch_transition = 1.0f;  // 1.0 = standing, 0.0 = crouched

    // Input state (sampled in this tick)
    float forward_input = 0.0f;      // -1, 0, 1
    float side_input = 0.0f;         // -1, 0, 1
    bool jump_pressed = false;
    bool sprint_pressed = false;
    bool crouch_pressed = false;

    // Camera/eye height
    float eye_height = 1.7f;

    // Entity states (for interpolation)
    // TODO: If using full ECS, store entity list here
    // std::vector<EntityState> entities;
};

} // namespace S67
