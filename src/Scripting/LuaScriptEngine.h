#pragma once

#include "Core/Base.h"
#include "Renderer/Entity.h"
#include <sol/sol.hpp>
#include <string>
#include <filesystem>
#include <optional>
#include <bitset>

namespace S67 {

    class LuaScriptEngine {
    public:
    public:
        static void Init();
        static void Shutdown();

        static void OnUpdate(Entity* entity, float ts);
        static void OnCreate(Entity* entity);
        
        static void BeginFrame(); // Updates input state

        static sol::state& GetState() { return s_State; }

    private:
        static void BindAPI();
        
    private:
        static sol::state s_State;
        static std::bitset<512> s_LastKeys;
        static std::bitset<512> s_JustPressed;
    };

}
