#pragma once

#include "Base.h"
#include "Logger.h"
#include <filesystem>

#ifdef S67_ENABLE_ASSERTS

    // Alternatively: #define S67_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { S67##type##_ERROR(msg, __VA_ARGS__); S67_DEBUGBREAK(); } }
    #define S67_ASSERT(x, ...) { if(!(x)) { S67_ERROR("Assertion Failed: {0}", __VA_ARGS__); __builtin_trap(); } }
    #define S67_CORE_ASSERT(x, ...) { if(!(x)) { S67_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __builtin_trap(); } }
#else
    #define S67_ASSERT(x, ...)
    #define S67_CORE_ASSERT(x, ...)
#endif
