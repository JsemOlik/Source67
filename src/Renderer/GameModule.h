#pragma once

#include "ScriptRegistry.h"

// Every external C++ module should implement this entry point
// to register its scripts into the engine.
// Example:
// extern "C" void InitGameModule(S67::ScriptRegistry* registry) {
//     registry->Register<MyCustomScript>("MyCustomScript");
// }

#ifdef WIN32
    #define GAME_MODULE_EXPORT __declspec(dllexport)
#else
    #define GAME_MODULE_EXPORT __attribute__((visibility("default")))
#endif

#define DEFINE_GAME_MODULE() \
    extern "C" GAME_MODULE_EXPORT void InitGameModule(S67::ScriptRegistry* registry)
