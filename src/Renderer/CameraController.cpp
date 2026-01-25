#include "CameraController.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"

namespace S67 {

    CameraController::CameraController(const Ref<PerspectiveCamera>& camera)
        : m_Camera(camera) {
    }

    void CameraController::OnUpdate(Timestep ts) {
        float speed = m_CameraTranslationSpeed * ts;

        if (Input::IsKeyPressed(S67_KEY_W))
            m_CameraPosition += m_Camera->GetForward() * speed;
        if (Input::IsKeyPressed(S67_KEY_S))
            m_CameraPosition -= m_Camera->GetForward() * speed;
        if (Input::IsKeyPressed(S67_KEY_A))
            m_CameraPosition -= m_Camera->GetRight() * speed;
        if (Input::IsKeyPressed(S67_KEY_D))
            m_CameraPosition += m_Camera->GetRight() * speed;
        if (Input::IsKeyPressed(S67_KEY_Q))
            m_CameraPosition.y -= speed;
        if (Input::IsKeyPressed(S67_KEY_E))
            m_CameraPosition.y += speed;

        m_Camera->SetPosition(m_CameraPosition);
    }

    void CameraController::OnEvent(Event& e) {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FN(CameraController::OnMouseMoved));
    }

    bool CameraController::OnMouseMoved(MouseMovedEvent& e) {
        if (m_FirstMouse) {
            m_LastMouseX = e.GetX();
            m_LastMouseY = e.GetY();
            m_FirstMouse = false;
        }

        float xOffset = e.GetX() - m_LastMouseX;
        float yOffset = m_LastMouseY - e.GetY(); // Reversed since y-coordinates go from bottom to top
        m_LastMouseX = e.GetX();
        m_LastMouseY = e.GetY();

        xOffset *= m_CameraRotationSpeed;
        yOffset *= m_CameraRotationSpeed;

        m_Camera->SetYaw(m_Camera->GetYaw() + xOffset);
        m_Camera->SetPitch(glm::clamp(m_Camera->GetPitch() + yOffset, -89.0f, 89.0f));

        return false;
    }

}
