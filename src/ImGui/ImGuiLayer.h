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
        void SetDarkThemeColors();
        void SetDraculaThemeColors();

        void SetBlockEvents(bool block) { m_BlockEvents = block; }

        void SaveLayout(const std::string& path = "");
        void LoadLayout(const std::string& path = "");

    private:
        bool m_BlockEvents = true;
    };

}
