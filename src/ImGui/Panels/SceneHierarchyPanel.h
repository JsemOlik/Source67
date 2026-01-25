#pragma once

#include "Core/Base.h"
#include "Renderer/Scene.h"
#include <imgui.h>

namespace S67 {

    class SceneHierarchyPanel {
    public:
        SceneHierarchyPanel() = default;
        SceneHierarchyPanel(const Scope<Scene>& context);

        void SetContext(const Scope<Scene>& context);

        void OnImGuiRender();

        Ref<Entity> GetSelectedEntity() const { return m_SelectionContext; }

    private:
        void DrawEntityNode(Ref<Entity> entity);
        void DrawProperties(Ref<Entity> entity);

    private:
        const Scope<Scene>* m_Context = nullptr;
        Ref<Entity> m_SelectionContext;
    };

}
