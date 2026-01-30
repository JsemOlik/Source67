#include "ScriptRegistry.h"
#include "Core/Logger.h"

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace S67 {

void ScriptRegistry::LoadModules(const std::filesystem::path &directory) {
  if (!std::filesystem::exists(directory))
    return;

  for (const auto &entry : std::filesystem::directory_iterator(directory)) {
    if (entry.is_regular_file()) {
      auto ext = entry.path().extension().string();
#ifdef WIN32
      if (ext == ".dll")
#else
      if (ext == ".dylib") // macOS
#endif
      {
        LoadModule(entry.path());
      }
    }
  }
}

void ScriptRegistry::LoadModule(const std::filesystem::path &filepath) {
  S67_CORE_INFO("Loading script module: {0}", filepath.string());

#ifdef WIN32
  HMODULE handle = LoadLibraryA(filepath.string().c_str());
  if (!handle) {
    S67_CORE_ERROR("Failed to load DLL: {0}", filepath.string());
    return;
  }

  using InitFunc = void (*)(ScriptRegistry *);
  InitFunc init = (InitFunc)GetProcAddress(handle, "InitGameModule");
  if (init) {
    m_IsLoadingModule = true;
    init(this);
    m_IsLoadingModule = false;
    m_ModuleHandles.push_back((void *)handle);
  } else {
    S67_CORE_ERROR("Module {0} missing InitGameModule entry point",
                   filepath.string());
    FreeLibrary(handle);
  }
#else
  void *handle = dlopen(filepath.string().c_str(), RTLD_NOW);
  if (!handle) {
    S67_CORE_ERROR("Failed to load dylib: {0}. Error: {1}", filepath.string(),
                   dlerror());
    return;
  }

  using InitFunc = void (*)(ScriptRegistry *);
  InitFunc init = (InitFunc)dlsym(handle, "InitGameModule");
  if (init) {
    m_IsLoadingModule = true;
    init(this);
    m_IsLoadingModule = false;
    m_ModuleHandles.push_back(handle);
  } else {
    S67_CORE_ERROR("Module {0} missing InitGameModule entry point",
                   filepath.string());
    dlclose(handle);
  }
#endif
}

void ScriptRegistry::UnloadModules() {
  // Remove scripts from registry
  for (const auto &name : m_DynamicScriptNames) {
    m_Registry.erase(name);
  }
  m_DynamicScriptNames.clear();

  for (void *handle : m_ModuleHandles) {
#ifdef WIN32
    FreeLibrary((HMODULE)handle);
#else
    dlclose(handle);
#endif
  }
  m_ModuleHandles.clear();
}

} // namespace S67
