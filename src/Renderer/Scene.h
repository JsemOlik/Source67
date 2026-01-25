#pragma once

#include "Entity.h"
#include "Camera.h"
#include <vector>

namespace S67 {

    class Scene {
    public:
        Scene() = default;

        void AddEntity(const Ref<Entity>& entity) { m_Entities.push_back(entity); }
        void Clear() { m_Entities.clear(); }
        const std::vector<Ref<Entity>>& GetEntities() const { return m_Entities; }

    private:
        std::vector<Ref<Entity>> m_Entities;
    };

}
