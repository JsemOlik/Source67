#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Renderer/VertexArray.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <string>

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

    struct Material {
        Ref<Texture2D> AlbedoMap;
        glm::vec2 Tiling = { 1.0f, 1.0f };
    };

    class Entity {
    public:
        Entity() = default;
        Entity(const std::string& name, const Ref<VertexArray>& va, const Ref<Shader>& shader, const Ref<Texture2D>& texture)
            : Name(name), Mesh(va), MaterialShader(shader) {
            Material.AlbedoMap = texture;
        }

        Transform Transform;
        Ref<VertexArray> Mesh;
        Ref<Shader> MaterialShader;
        Material Material;

        JPH::BodyID PhysicsBody;
        std::string Name = "Entity";
        std::string MeshPath = "Cube";
        bool Collidable = true;
    };

}
