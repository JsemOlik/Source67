#pragma once

// Example enemy component for Game DLL

class EnemyComponent {
public:
    EnemyComponent() = default;
    ~EnemyComponent() = default;

    void OnCreate();
    void OnUpdate(float deltaTime);
    void OnDestroy();

private:
    float m_Speed = 3.0f;
    float m_DetectionRadius = 10.0f;
};
