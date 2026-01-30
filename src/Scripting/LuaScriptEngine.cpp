#include "LuaScriptEngine.h"
#include "Core/Application.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"
#include "Renderer/HUDRenderer.h"
#include "Physics/PhysicsSystem.h"
#include "Core/Logger.h"

namespace S67 {

    sol::state LuaScriptEngine::s_State;

    void LuaScriptEngine::Init() {
        s_State.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math, sol::lib::string);
        BindAPI();
        S67_CORE_INFO("LuaScriptEngine initialized");
    }

    void LuaScriptEngine::Shutdown() {
        // sol::state cleans up automatically
    }

    void LuaScriptEngine::BindAPI() {
        // Math types
        s_State.new_usertype<glm::vec2>("vec2",
            sol::constructors<glm::vec2(), glm::vec2(float), glm::vec2(float, float)>(),
            "x", &glm::vec2::x,
            "y", &glm::vec2::y
        );

        s_State.new_usertype<glm::vec3>("vec3",
            sol::constructors<glm::vec3(), glm::vec3(float), glm::vec3(float, float, float)>(),
            "x", &glm::vec3::x,
            "y", &glm::vec3::y,
            "z", &glm::vec3::z
        );

        s_State.new_usertype<glm::vec4>("vec4",
            sol::constructors<glm::vec4(), glm::vec4(float), glm::vec4(float, float, float, float)>(),
            "x", &glm::vec4::x,
            "y", &glm::vec4::y,
            "z", &glm::vec4::z,
            "w", &glm::vec4::w
        );

        // Transform
        s_State.new_usertype<Transform>("Transform",
            "position", &Transform::Position,
            "rotation", &Transform::Rotation,
            "scale", &Transform::Scale
        );

        // Entity
        s_State.new_usertype<Entity>("Entity",
            "transform", &Entity::Transform,
            "hasTag", &Entity::HasTag,
            "getName", [](Entity& self) { return self.Name; }
        );

        // Core API Functions (Stupid Simple)
        s_State.set_function("printHUD", [](const std::string& text, sol::optional<glm::vec4> color) {
            HUDRenderer::QueueString(text, color.value_or(glm::vec4(1.0f)));
        });

        s_State.set_function("setText", [](const std::string& id, const std::string& text, sol::optional<glm::vec2> pos, sol::optional<float> scale, sol::optional<glm::vec4> color) {
            HUDRenderer::SetText(id, text, pos.value_or(glm::vec2(0.5f, 0.1f)), scale.value_or(3.0f), color.value_or(glm::vec4(1.0f)));
        });

        s_State.set_function("clearText", [](const std::string& id) {
            HUDRenderer::ClearText(id);
        });

        s_State.set_function("findEntity", [](const std::string& name) {
            return Application::Get().GetScene().FindEntityByName(name).get();
        });

        s_State.set_function("isKeyPressed", [](int key) {
            return Input::IsKeyPressed(key);
        });

        // Key Codes
        s_State["KEY_E"] = S67_KEY_E;
        s_State["KEY_F"] = S67_KEY_F;
        s_State["KEY_SPACE"] = S67_KEY_SPACE;
        // ... add more if needed, or bind the whole enum if we had one

        // Global Raycast (Player context)
        s_State.set_function("raycast", [](float distance) -> Entity* {
            // Re-using the logic from ScriptableEntity::Raycast for now
            // Future: Move this logic to a centralized PhysicsHelper
            auto player = Application::Get().GetScene().FindEntityByName("Player");
            if (!player) return nullptr;

            auto& transform = player->Transform;
            glm::vec3 origin = transform.Position + glm::vec3(0.0f, 1.7f, 0.0f);
            float pitch = glm::radians(transform.Rotation.x);
            float yaw = glm::radians(transform.Rotation.y - 90.0f);
            glm::vec3 direction;
            direction.x = cos(pitch) * cos(yaw);
            direction.y = sin(pitch);
            direction.z = cos(pitch) * sin(yaw);
            direction = glm::normalize(direction);

            JPH::BodyID hitBody = PhysicsSystem::Raycast(origin, direction, distance);
            if (!hitBody.IsInvalid()) {
                auto& bodyInterface = PhysicsSystem::GetBodyInterface();
                return (Entity*)bodyInterface.GetUserData(hitBody);
            }
            return nullptr;
        });
    }

    void LuaScriptEngine::OnCreate(Entity* entity) {
        for (auto& script : entity->LuaScripts) {
            if (script.FilePath.empty()) continue;

            auto result = s_State.safe_script_file(script.FilePath, sol::script_pass_on_error);
            if (!result.valid()) {
                sol::error err = result;
                S67_CORE_ERROR("Failed to load Lua script {0}: {1}", script.FilePath, err.what());
                continue;
            }

            // Call onCreate if it exists
            sol::protected_function onCreateFunc = s_State["onCreate"];
            if (onCreateFunc.valid()) {
                s_State["self"] = entity; // Set context
                auto callResult = onCreateFunc();
                if (!callResult.valid()) {
                    sol::error err = callResult;
                    S67_CORE_ERROR("Lua onCreate error in {0}: {1}", script.FilePath, err.what());
                }
            }
        }
    }

    void LuaScriptEngine::OnUpdate(Entity* entity, float ts) {
        for (auto& script : entity->LuaScripts) {
            if (script.FilePath.empty()) continue;

            // Call onUpdate if it exists
            sol::protected_function onUpdateFunc = s_State["onUpdate"];
            if (onUpdateFunc.valid()) {
                s_State["self"] = entity; // Set context
                auto callResult = onUpdateFunc(ts);
                if (!callResult.valid()) {
                    sol::error err = callResult;
                    S67_CORE_ERROR("Lua onUpdate error in {0}: {1}", script.FilePath, err.what());
                }
            }
        }
    }

}
