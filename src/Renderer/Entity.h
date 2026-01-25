#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Renderer/VertexArray.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"

namespace S67 {

    struct Transform {
        glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f }; // Euler angles
        glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

        glm::mat4 GetTransform() const {
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.x), { 1, 0, 0 }) *
                                 glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.y), { 0, 1, 0 }) *
                                 glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.z), { 0, 0, 1 });

            return glm::translate(glm::mat4(1.0f), Position) * rotation * glm::scale(glm::mat4(1.0f), Scale);
        }
    };

    class Entity {
    public:
        Entity() = default;
        Entity(const Ref<VertexArray>& va, const Ref<Shader>& shader, const Ref<Texture2D>& texture)
            : Mesh(va), MaterialShader(shader), MaterialTexture(texture) {}

        Transform Transform;
        Ref<VertexArray> Mesh;
        Ref<Shader> MaterialShader;
        Ref<Texture2D> MaterialTexture;
    };

}
