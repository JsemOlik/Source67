#include "../../src/Core/PakSystem.h"
#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

void PrintUsage() {
  std::cout << "Usage: asset_packer -i <input_dir> -o <output_file> [-v] "
               "[--include-lua]"
            << std::endl;
}

S67::AssetType DetermineType(const fs::path &path) {
  auto ext = path.extension().string();
  if (ext == ".png" || ext == ".jpg" || ext == ".tga")
    return S67::AssetType::ASSET_TEXTURE;
  if (ext == ".obj" || ext == ".fbx" || ext == ".gltf")
    return S67::AssetType::ASSET_MODEL;
  if (ext == ".s67")
    return S67::AssetType::ASSET_SCENE;
  if (ext == ".glsl")
    return S67::AssetType::ASSET_SHADER;
  if (ext == ".ttf")
    return S67::AssetType::ASSET_FONT;
  if (ext == ".lua")
    return S67::AssetType::ASSET_LUA_SCRIPT;
  if (ext == ".json")
    return S67::AssetType::ASSET_CONFIG_JSON;
  return S67::AssetType::ASSET_UNKNOWN;
}

int main(int argc, char *argv[]) {
  std::string inputDir = "assets";
  std::string outputFile = "GameAssets.apak";
  bool verbose = false;
  bool includeLua = true;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-i" && i + 1 < argc)
      inputDir = argv[++i];
    else if (arg == "-o" && i + 1 < argc)
      outputFile = argv[++i];
    else if (arg == "-v")
      verbose = true;
    else if (arg == "--include-lua")
      includeLua = true;
  }

  if (!fs::exists(inputDir) || !fs::is_directory(inputDir)) {
    std::cerr << "Input directory does not exist: " << inputDir << std::endl;
    return 1;
  }

  std::cout << "Packing assets from " << inputDir << " to " << outputFile
            << "..." << std::endl;

  S67::PakWriter writer(outputFile);

  for (const auto &entry : fs::recursive_directory_iterator(inputDir)) {
    if (entry.is_regular_file()) {
      auto path = entry.path();
      auto type = DetermineType(path);

      if (type == S67::AssetType::ASSET_LUA_SCRIPT && !includeLua)
        continue;

      if (verbose) {
        std::cout << "  Adding: " << path.string() << " (Type: " << (int)type
                  << ")" << std::endl;
      }
      writer.AddFile(path.string(), type);
    }
  }

  if (writer.Write()) {
    std::cout << "Successfully created " << outputFile << std::endl;
    return 0;
  } else {
    std::cerr << "Failed to create " << outputFile << std::endl;
    return 1;
  }
}
