#include "LuaScriptEngine.h"
#include "Core/Application.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"
#include "Renderer/HUDRenderer.h"
#include "Physics/PhysicsSystem.h"
#include "Core/Logger.h"

namespace S67 {

    sol::state LuaScriptEngine::s_State;
    std::bitset<512> LuaScriptEngine::s_LastKeys;
    std::bitset<512> LuaScriptEngine::s_JustPressed;

    void LuaScriptEngine::Init() {
        s_State.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math, sol::lib::string);
        BindAPI();
        S67_CORE_INFO("LuaScriptEngine initialized");
    }

    void LuaScriptEngine::BeginFrame() {
        s_JustPressed.reset();
        // Optimize: Only check range of common keys or 0-350 (GLFW_KEY_LAST is 348)
        for (int i = 32; i <= 348; i++) {
             bool pressed = Input::IsKeyPressed(i);
             if (pressed && !s_LastKeys.test(i)) {
                 s_JustPressed.set(i);
             }
             s_LastKeys.set(i, pressed);
        }
    }

    void LuaScriptEngine::Shutdown() {
        // sol::state cleans up automatically
    }

    void LuaScriptEngine::BindAPI() {
        // Math types
        s_State.new_usertype<glm::vec2>("Vec2",
            sol::constructors<glm::vec2(), glm::vec2(float), glm::vec2(float, float)>(),
            "x", &glm::vec2::x,
            "y", &glm::vec2::y,
            sol::meta_function::addition, [](const glm::vec2& a, const glm::vec2& b) { return a + b; },
            sol::meta_function::subtraction, [](const glm::vec2& a, const glm::vec2& b) { return a - b; },
            sol::meta_function::multiplication, [](const glm::vec2& a, float b) { return a * b; }
        );

        s_State.set_function("vec2", sol::overload(
            [](float c) { return glm::vec2(c); },
            [](float x, float y) { return glm::vec2(x, y); }
        ));

        s_State.new_usertype<glm::vec3>("Vec3",
            sol::constructors<glm::vec3(), glm::vec3(float), glm::vec3(float, float, float)>(),
            "x", &glm::vec3::x,
            "y", &glm::vec3::y,
            "z", &glm::vec3::z,
            sol::meta_function::addition, [](const glm::vec3& a, const glm::vec3& b) { return a + b; },
            sol::meta_function::subtraction, [](const glm::vec3& a, const glm::vec3& b) { return a - b; },
            sol::meta_function::multiplication, [](const glm::vec3& a, float b) { return a * b; }
        );

        s_State.set_function("vec3", sol::overload(
            [](float c) { return glm::vec3(c); },
            [](float x, float y, float z) { return glm::vec3(x, y, z); }
        ));

        s_State.new_usertype<glm::vec4>("Vec4",
            sol::constructors<glm::vec4(), glm::vec4(float), glm::vec4(float, float, float, float)>(),
            "x", &glm::vec4::x,
            "y", &glm::vec4::y,
            "z", &glm::vec4::z,
            "w", &glm::vec4::w,
            sol::meta_function::addition, [](const glm::vec4& a, const glm::vec4& b) { return a + b; },
            sol::meta_function::subtraction, [](const glm::vec4& a, const glm::vec4& b) { return a - b; },
            sol::meta_function::multiplication, [](const glm::vec4& a, float b) { return a * b; }
        );

        s_State.set_function("vec4", sol::overload(
            [](float c) { return glm::vec4(c); },
            [](float x, float y, float z, float w) { return glm::vec4(x, y, z, w); }
        ));

        // Transform
        s_State.new_usertype<Transform>("Transform",
            "position", &Transform::Position,
            "rotation", &Transform::Rotation,
            "scale", &Transform::Scale
        );

        // Entity
        s_State.new_usertype<Entity>("Entity",
            "transform", sol::property([](Entity& self) -> Transform& { return self.Transform; }, [](Entity& self, const Transform& t) { self.Transform = t; }),
            "hasTag", &Entity::HasTag,
            "getName", [](Entity& self) { return self.Name; },
            "getPosition", [](Entity& self) { return self.Transform.Position; },
            "setPosition", [](Entity& self, const glm::vec3& pos) { 
                self.Transform.Position = pos; 
                if (!self.PhysicsBody.IsInvalid()) {
                     PhysicsSystem::GetBodyInterface().SetPosition(self.PhysicsBody, JPH::RVec3(pos.x, pos.y, pos.z), JPH::EActivation::Activate);
                }
            },
            "getLinearVelocity", [](Entity& self) -> glm::vec3 {
                if (!self.PhysicsBody.IsInvalid()) {
                    JPH::RVec3 v = PhysicsSystem::GetBodyInterface().GetLinearVelocity(self.PhysicsBody);
                    return glm::vec3(v.GetX(), v.GetY(), v.GetZ());
                }
                return glm::vec3(0.0f);
            },
            "setLinearVelocity", [](Entity& self, const glm::vec3& velocity) {
                if (!self.PhysicsBody.IsInvalid()) {
                    PhysicsSystem::GetBodyInterface().SetLinearVelocity(self.PhysicsBody, JPH::RVec3(velocity.x, velocity.y, velocity.z));
                }
            },
            "setAnchored", [](Entity& self, bool anchored) {
                self.Anchored = anchored;
                if (!self.PhysicsBody.IsInvalid()) {
                    PhysicsSystem::GetBodyInterface().SetMotionType(self.PhysicsBody, anchored ? JPH::EMotionType::Kinematic : JPH::EMotionType::Dynamic, JPH::EActivation::Activate);
                }
            },
            "isAnchored", &Entity::Anchored,
            "setRotation", [](Entity& self, const glm::vec3& euler) {
                self.Transform.Rotation = euler;
                if (!self.PhysicsBody.IsInvalid()) {
                    float x = glm::radians(euler.x);
                    float y = glm::radians(euler.y);
                    float z = glm::radians(euler.z);
                    PhysicsSystem::GetBodyInterface().SetRotation(self.PhysicsBody, JPH::Quat::sEulerAngles(JPH::Vec3(x, y, z)), JPH::EActivation::Activate);
                }
            },
            "getRotation", [](Entity& self) -> glm::vec3 {
                return self.Transform.Rotation;
            }
        );

        // Core API Functions (Stupid Simple)
        s_State.set_function("printHUD", [](const std::string& text, sol::optional<glm::vec4> color) {
            HUDRenderer::QueueString(text, color.value_or(glm::vec4(1.0f)));
        });

        s_State.set_function("log", [](const std::string& message) {
            S67_CORE_INFO("[Lua] {0}", message);
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

        s_State.set_function("isKeyHeld", [](int key) {
            return Input::IsKeyPressed(key);
        });

        s_State.set_function("isKeyPressed", [](int key) {
            if (key >= 0 && key < 512) return s_JustPressed.test(key);
            return false;
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

            std::string scriptPath = Application::Get().ResolveAssetPath(script.FilePath).string();
            // Fallback: If not found, try raw path (compatibility)
            if (!std::filesystem::exists(scriptPath)) scriptPath = script.FilePath;

            // Create Sandboxed Environment
            // This ensures each script has its own globals (like 'time', 'self', etc.)
            auto env = std::make_shared<sol::environment>(s_State, sol::create, s_State.globals());
            script.Environment = env;
            
            // Inject 'self' into the environment
            (*env)["self"] = entity;

            // Load Script into Environment
            auto result = s_State.safe_script_file(scriptPath, *env, sol::script_pass_on_error);
            if (!result.valid()) {
                sol::error err = result;
                S67_CORE_ERROR("Failed to load Lua script {0}: {1}", script.FilePath, err.what());
                continue;
            }

            if (std::filesystem::exists(scriptPath)) {
                script.LastWriteTime = std::filesystem::last_write_time(scriptPath);
            }

            // Call onCreate if it exists in the environment
            sol::protected_function onCreateFunc = (*env)["onCreate"];
            if (onCreateFunc.valid()) {
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
            
            // If environment is missing (e.g. added at runtime?), try to create it? 
            // Better to rely on OnCreate being called first. Scene::InstantiateScripts handles this.
            if (!script.Environment) continue;

            auto env = std::static_pointer_cast<sol::environment>(script.Environment);

            // Hot Reloading Check
            std::string scriptPath = Application::Get().ResolveAssetPath(script.FilePath).string();
            if (!std::filesystem::exists(scriptPath)) scriptPath = script.FilePath;

            if (std::filesystem::exists(scriptPath)) {
                auto lastWriteTime = std::filesystem::last_write_time(scriptPath);
                if (lastWriteTime > script.LastWriteTime) {
                    // Hot Reload: Re-run script into the SAME environment to update functions but keep state?
                    // Or new environment? If we want data persistence (like 'time'), same env is better.
                    auto result = s_State.safe_script_file(scriptPath, *env, sol::script_pass_on_error);
                    if (result.valid()) {
                         script.LastWriteTime = lastWriteTime;
                         S67_CORE_INFO("Hot Reloaded Lua script: {0}", script.FilePath);
                    } else {
                         sol::error err = result;
                         S67_CORE_ERROR("Failed to hot reload Lua script {0}: {1}", script.FilePath, err.what());
                    }
                }
            }

            // Call onUpdate if it exists
            sol::protected_function onUpdateFunc = (*env)["onUpdate"];
            if (onUpdateFunc.valid()) {
                auto callResult = onUpdateFunc(ts);
                if (!callResult.valid()) {
                    sol::error err = callResult;
                    S67_CORE_ERROR("Lua onUpdate error in {0}: {1}", script.FilePath, err.what());
                }
            }
        }
    }

}
