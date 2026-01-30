#pragma once

#include "Core/Base.h"
#include "Renderer/Entity.h"
#include <sol/sol.hpp>
#include <string>
#include <filesystem>
#include <optional>

namespace S67 {

    class LuaScriptEngine {
    public:
        static void Init();
        static void Shutdown();

        static void OnUpdate(Entity* entity, float ts);
        static void OnCreate(Entity* entity);

        static sol::state& GetState() { return s_State; }

    private:
        static void BindAPI();
        
    private:
        static sol::state s_State;
    };

}
