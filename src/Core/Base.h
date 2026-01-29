#pragma once

#include <memory>

#ifdef S67_ENABLE_ASSERTS
#define S67_ASSERT(x, ...)                                                     \
  {                                                                            \
    if (!(x)) {                                                                \
      S67_ERROR("Assertion Failed: {0}", __VA_ARGS__);                         \
      __builtin_trap();                                                        \
    }                                                                          \
  }
#define S67_CORE_ASSERT(x, ...)                                                \
  {                                                                            \
    if (!(x)) {                                                                \
      S67_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__);                    \
      __builtin_trap();                                                        \
    }                                                                          \
  }
#else
#define S67_ASSERT(x, ...)
#define S67_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define BIND_EVENT_FN(fn)                                                      \
  [this](auto &&...args) -> decltype(auto) {                                   \
    return this->fn(std::forward<decltype(args)>(args)...);                    \
  }

namespace S67 {

template <typename T> using Scope = std::unique_ptr<T>;

template <typename T, typename... Args>
constexpr Scope<T> CreateScope(Args &&...args) {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T> using Ref = std::shared_ptr<T>;

template <typename T, typename... Args>
constexpr Ref<T> CreateRef(Args &&...args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

} // namespace S67
