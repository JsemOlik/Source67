#pragma once

#include "Entity.h"
#include "Camera.h"
#include <vector>
#include <algorithm>

namespace S67 {

    class Scene {
    public:
        Scene() = default;

        void AddEntity(const Ref<Entity>& entity) { m_Entities.push_back(entity); }
        void RemoveEntity(const Ref<Entity>& entity) {
            auto it = std::find(m_Entities.begin(), m_Entities.end(), entity);
            if (it != m_Entities.end()) m_Entities.erase(it);
        }
        void Clear() { m_Entities.clear(); }
        const std::vector<Ref<Entity>>& GetEntities() const { return m_Entities; }

    private:
        std::vector<Ref<Entity>> m_Entities;
    };

}
