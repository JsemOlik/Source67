#include "PlayerComponent.h"
#include <iostream>

void PlayerComponent::OnCreate() {
    std::cout << "[PlayerComponent] Created - Speed: " << m_Speed 
              << ", Health: " << m_Health << std::endl;
}

void PlayerComponent::OnUpdate(float deltaTime) {
    // Player update logic
    // In production: Handle input, movement, physics, etc.
}

void PlayerComponent::OnDestroy() {
    std::cout << "[PlayerComponent] Destroyed" << std::endl;
}
