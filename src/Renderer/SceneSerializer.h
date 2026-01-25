#pragma once

#include "Scene.h"

namespace S67 {

    class SceneSerializer {
    public:
        SceneSerializer(Scene* scene);

        void Serialize(const std::string& filepath);
        bool Deserialize(const std::string& filepath);

    private:
        Scene* m_Scene;
    };

}
