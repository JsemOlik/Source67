#include "AssetCooker.h"
#include "Core/Logger.h"
#include <fstream>
#include <nlohmann/json.hpp>

namespace S67 {

void AssetCooker::Cook(const CookingOptions &options) {
  S67_CORE_INFO("Starting asset cooking...");
  S67_CORE_INFO("Source: {0}", options.SourceDir.string());
  S67_CORE_INFO("Output: {0}", options.OutputDir.string());

  if (!std::filesystem::exists(options.OutputDir)) {
    std::filesystem::create_directories(options.OutputDir);
  }

  for (auto const &dir_entry :
       std::filesystem::recursive_directory_iterator(options.SourceDir)) {
    if (dir_entry.is_regular_file()) {
      CookFile(dir_entry.path(), options);
    }
  }

  S67_CORE_INFO("Asset cooking completed.");
}

void AssetCooker::CookFile(const std::filesystem::path &path,
                           const CookingOptions &options) {
  std::string ext = path.extension().string();
  std::filesystem::path relativePath =
      std::filesystem::relative(path, options.SourceDir);
  std::filesystem::path outPath = options.OutputDir / relativePath;

  std::filesystem::create_directories(outPath.parent_path());

  if (ext == ".png" || ext == ".jpg" || ext == ".tga") {
    CookTexture(path, outPath);
  } else if (ext == ".obj" || ext == ".stl") {
    CookMesh(path, outPath);
  } else if (ext == ".s67scene") {
    CookScene(path, outPath);
  } else {
    // Just copy other files (shaders, scripts, etc.)
    std::filesystem::copy_file(
        path, outPath, std::filesystem::copy_options::overwrite_existing);
  }
}

void AssetCooker::CookTexture(const std::filesystem::path &path,
                              const std::filesystem::path &outPath) {
  // For now, just copy. In a real engine, we'd convert to KTX2/DDS.
  S67_CORE_INFO("Cooking Texture: {0}", path.filename().string());
  std::filesystem::copy_file(path, outPath,
                             std::filesystem::copy_options::overwrite_existing);
}

void AssetCooker::CookMesh(const std::filesystem::path &path,
                           const std::filesystem::path &outPath) {
  // For now, just copy. In a real engine, we'd convert to a binary format.
  S67_CORE_INFO("Cooking Mesh: {0}", path.filename().string());
  std::filesystem::copy_file(path, outPath,
                             std::filesystem::copy_options::overwrite_existing);
}

void AssetCooker::CookScene(const std::filesystem::path &path,
                            const std::filesystem::path &outPath) {
  S67_CORE_INFO("Cooking Scene: {0}", path.filename().string());

  // Load scene JSON and potentially process it (e.g., fix paths)
  try {
    std::ifstream f(path);
    nlohmann::json data = nlohmann::json::parse(f);

    // For now, we just write it back.
    // We could strip editor-only data here.

    std::ofstream out(outPath);
    out << data.dump(4);
  } catch (const std::exception &e) {
    S67_CORE_ERROR("Failed to cook scene {0}: {1}", path.string(), e.what());
  }
}

} // namespace S67
