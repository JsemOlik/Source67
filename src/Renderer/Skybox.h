#pragma once

#include "Renderer/VertexArray.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"
#include "Renderer/Camera.h"

namespace S67 {

    class Skybox {
    public:
        Skybox(const std::string& texturePath);
        ~Skybox() = default;

        void Draw(const Camera& camera);

    private:
        Ref<VertexArray> m_VertexArray;
        Ref<Shader> m_Shader;
        Ref<Texture2D> m_Texture;
    };

}
