#pragma once

#include "Renderer/Entity.h"
#include "Renderer/ScriptableEntity.h"
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace S67 {

class ScriptRegistry {
public:
  using InstantiateFunc = std::function<ScriptableEntity *()>;

  static ScriptRegistry &Get() {
    static ScriptRegistry instance;
    return instance;
  }

  template <typename T> void Register(const std::string &name) {
    m_Registry[name] = []() { return static_cast<ScriptableEntity *>(new T()); };
  }

  ScriptableEntity *Instantiate(const std::string &name) {
    if (m_Registry.find(name) != m_Registry.end()) {
      return m_Registry[name]();
    }
    return nullptr;
  }

  const std::map<std::string, InstantiateFunc> &GetAvailableScripts() const {
    return m_Registry;
  }

  void Bind(const std::string &name, NativeScriptComponent &nsc) {
    if (m_Registry.find(name) != m_Registry.end()) {
      nsc.Name = name;
      nsc.InstantiateScript = [](NativeScriptComponent *nsc_ptr) -> ScriptableEntity * {
          return ScriptRegistry::Get().Instantiate(nsc_ptr->Name);
      };
      nsc.DestroyScript = [](NativeScriptComponent *nsc_ptr) {
        delete nsc_ptr->Instance;
        nsc_ptr->Instance = nullptr;
      };
    }
  }

private:
  std::map<std::string, InstantiateFunc> m_Registry;
};

// Helper macro for auto-registration
#define REGISTER_SCRIPT(T) \
    static inline bool T##_registered = []() { \
        S67::ScriptRegistry::Get().Register<T>(#T); \
        return true; \
    }()

} // namespace S67
