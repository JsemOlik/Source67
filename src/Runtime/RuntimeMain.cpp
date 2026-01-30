#include "Core/Application.h"
#include "Core/Logger.h"
#include "Core/VFS.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>


using namespace S67;

int main(int argc, char **argv) {
  // 1. Init VFS
  VFS::Init();

  // 2. Resolve paths
  std::filesystem::path exePath =
      std::filesystem::absolute(argv[0]).parent_path();
  std::filesystem::path pakPath = exePath / "assets.pak";
  std::filesystem::path manifestPath = exePath / "manifest.source";

  if (!std::filesystem::exists(pakPath)) {
    std::cerr << "Error: assets.pak not found!" << std::endl;
    return 1;
  }

  // 3. Mount Pak
  VFS::Mount(pakPath.string(), "");

  // 4. Load Manifest
  std::string projectName = "Source67 Game";
  if (std::filesystem::exists(manifestPath)) {
    try {
      std::ifstream f(manifestPath);
      nlohmann::json data = nlohmann::json::parse(f);
      projectName = data.value("ProjectName", projectName);
    } catch (...) {
    }
  }

  // 5. Run Application (Runtime Mode)
  // Application app(argv[0]);
  // app.InitRuntime(projectName); // We'll need to implement a runtime-only
  // init app.Run();

  std::cout << "Starting game: " << projectName << std::endl;
  std::cout << "Assets loaded from: " << pakPath.string() << std::endl;

  // For now, since Application is heavily tied to ImGui/Editor,
  // a true runtime entry point would need a simplified Application.
  // But we've restructured the build system to allow separating these.

  return 0;
}
