#pragma once

// Example player component for Game DLL
// In production, this would inherit from S67::ScriptableEntity

class PlayerComponent {
public:
    PlayerComponent() = default;
    ~PlayerComponent() = default;

    void OnCreate();
    void OnUpdate(float deltaTime);
    void OnDestroy();

private:
    float m_Speed = 5.0f;
    float m_Health = 100.0f;
};
