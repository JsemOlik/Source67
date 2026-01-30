#include "EnemyComponent.h"
#include <iostream>

void EnemyComponent::OnCreate() {
    std::cout << "[EnemyComponent] Created - Speed: " << m_Speed 
              << ", Detection Radius: " << m_DetectionRadius << std::endl;
}

void EnemyComponent::OnUpdate(float deltaTime) {
    // Enemy AI logic
    // In production: Pathfinding, state machine, etc.
}

void EnemyComponent::OnDestroy() {
    std::cout << "[EnemyComponent] Destroyed" << std::endl;
}
