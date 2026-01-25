#pragma once

#include "Camera.h"
#include "Core/Timestep.h"
#include "Events/Event.h"
#include "Events/MouseEvent.h"

namespace S67 {

    class CameraController {
    public:
        CameraController(const Ref<PerspectiveCamera>& camera);

        void OnUpdate(Timestep ts);
        void OnEvent(Event& e);

        void SetRotationEnabled(bool enabled) { m_RotationEnabled = enabled; }
        void SetFirstMouse(bool first) { m_FirstMouse = first; }

        Ref<PerspectiveCamera> GetCamera() { return m_Camera; }

    private:
        bool OnMouseMoved(MouseMovedEvent& e);
        bool OnMouseScrolled(MouseScrolledEvent& e);

    private:
        Ref<PerspectiveCamera> m_Camera;

        glm::vec3 m_CameraPosition = { 0.0f, 2.0f, 8.0f };
        float m_CameraTranslationSpeed = 5.0f;
        float m_CameraRotationSpeed = 0.1f;

        float m_LastMouseX = 0.0f, m_LastMouseY = 0.0f;
        bool m_FirstMouse = true;
        bool m_RotationEnabled = true;
    };

}
