#include "AssetProcessor.h"
#include "Core/Logger.h"
#include "Core/PakSystem.h"
#include <filesystem>
#include <iostream>


namespace fs = std::filesystem;

void PrintUsage() {
  std::cout << "Source67 Builder Tool\n";
  std::cout << "Usage:\n";
  std::cout
      << "  Source67Builder package <input_assets_dir> <output_pak_file>\n";
  std::cout << "  Source67Builder build <project_file> (Placeholder)\n";
}

int main(int argc, char **argv) {
  S67::Logger::Init();

  if (argc < 2) {
    PrintUsage();
    return 1;
  }

  std::string command = argv[1];

  if (command == "package") {
    if (argc < 4) {
      S67_CORE_ERROR("Missing arguments for package command.");
      PrintUsage();
      return 1;
    }

    fs::path inputDir = argv[2];
    std::string outputPak = argv[3];

    if (!fs::exists(inputDir)) {
      S67_CORE_ERROR("Input directory {0} does not exist", inputDir.string());
      return 1;
    }

    S67::PakWriter writer(outputPak);
    S67::TextureProcessor texProc;
    S67::MeshProcessor meshProc;
    S67::ShaderProcessor shaderProc;
    S67::LevelProcessor levelProc;

    S67_CORE_INFO("Packaging assets from {0} to {1}...", inputDir.string(),
                  outputPak);

    for (const auto &entry : fs::recursive_directory_iterator(inputDir)) {
      if (entry.is_directory())
        continue;

      fs::path path = entry.path();
      std::string ext = path.extension().string();
      S67::ProcessedAsset asset;
      bool processed = false;

      if (ext == ".png" || ext == ".jpg" || ext == ".tga") {
        processed = texProc.Process(path, asset);
      } else if (ext == ".obj" || ext == ".stl") {
        processed = meshProc.Process(path, asset);
      } else if (ext == ".glsl") {
        processed = shaderProc.Process(path, asset);
      } else if (ext == ".s67") {
        processed = levelProc.Process(path, asset);
      } else {
        // For other files, just bundle them raw
        S67_CORE_INFO("Bundling raw file: {0}", path.string());
        writer.AddFile(fs::relative(path, inputDir).generic_string(),
                       path.string());
        continue;
      }

      if (processed) {
        std::string relPath = fs::relative(path, inputDir).generic_string();
        // Change extension for processed assets if needed, or keep same for
        // compatibility
        writer.AddFile(relPath, asset.Data.data(), (uint32_t)asset.Data.size());
        S67_CORE_INFO("Processed and added: {0}", relPath);
      }
    }

    if (writer.Write()) {
      S67_CORE_INFO("Package created successfully!");
    } else {
      S67_CORE_ERROR("Failed to create package.");
      return 1;
    }
  } else {
    S67_CORE_ERROR("Unknown command: {0}", command);
    PrintUsage();
    return 1;
  }

  return 0;
}
