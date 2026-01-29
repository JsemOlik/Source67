#include "Core/Application.h"
#include "Core/Logger.h"
#include <memory>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32
int main(int argc, char **argv);

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline,
                     int cmdshow) {
  return main(__argc, __argv);
}
#endif

int main(int argc, char **argv) {
#ifdef _WIN32
  // Per-Monitor V2 awareness is supported since Windows 10, version 1703.
  // It handles automatic scaling of non-client area (title bar), menu, and
  // common controls.
  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#endif
  // Debug startup
  printf("DEBUG: Starting Main...\n");
  fflush(stdout);

  S67::Logger::Init();
  printf("DEBUG: Logger Initialized\n");
  fflush(stdout);

  S67_CORE_INFO("Source67 Engine Initialized");
  S67_CORE_INFO("Command line: argc={0}", argc);
  for (int i = 0; i < argc; i++) {
    S67_CORE_INFO("argv[{0}] = {1}", i, argv[i]);
  }

  auto app =
      std::make_unique<S67::Application>(argv[0], argc > 1 ? argv[1] : "");
  app->Run();
  // No delete needed - automatic cleanup

  return 0;
}