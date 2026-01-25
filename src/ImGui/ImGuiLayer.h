#pragma once

#include "Core/Base.h"
#include "Events/Event.h"

namespace S67 {

    class ImGuiLayer {
    public:
        void OnAttach();
        void OnDetach();
        void OnEvent(Event& e);

        void Begin();
        void End();

        void SetBlockEvents(bool block) { m_BlockEvents = block; }

    private:
        bool m_BlockEvents = true;
    };

}
