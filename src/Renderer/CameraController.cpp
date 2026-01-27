#include "CameraController.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"
#include <filesystem>
#include <utility>


namespace S67 {

CameraController::CameraController(const Ref<PerspectiveCamera> &camera)
    : m_Camera(camera) {}

void CameraController::OnUpdate(Timestep ts) {
  m_CameraPosition = m_Camera->GetPosition();
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

  // Rotation
  if (m_RotationEnabled) {
    std::pair<float, float> mousePos = Input::GetMousePosition();
    float x = mousePos.first;
    float y = mousePos.second;

    if (m_FirstMouse) {
      m_LastMouseX = x;
      m_LastMouseY = y;
      m_FirstMouse = false;
    }

    float xOffset = x - m_LastMouseX;
    float yOffset = m_LastMouseY - y;
    m_LastMouseX = x;
    m_LastMouseY = y;

    xOffset *= m_CameraRotationSpeed;
    yOffset *= m_CameraRotationSpeed;

    m_Camera->SetYaw(m_Camera->GetYaw() + xOffset);
    m_Camera->SetPitch(
        glm::clamp(m_Camera->GetPitch() + yOffset, -89.0f, 89.0f));
  }
}

void CameraController::OnEvent(Event &e) {
  // Events no longer used for rotation to improve robustness
}

} // namespace S67
