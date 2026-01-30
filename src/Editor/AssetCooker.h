#pragma once

#include <filesystem>
#include <string>
#include <vector>


namespace S67 {

class AssetCooker {
public:
  struct CookingOptions {
    std::filesystem::path SourceDir;
    std::filesystem::path OutputDir;
    bool Compress = false;
  };

  static void Cook(const CookingOptions &options);

private:
  static void CookFile(const std::filesystem::path &path,
                       const CookingOptions &options);
  static void CookTexture(const std::filesystem::path &path,
                          const std::filesystem::path &outPath);
  static void CookMesh(const std::filesystem::path &path,
                       const std::filesystem::path &outPath);
  static void CookScene(const std::filesystem::path &path,
                        const std::filesystem::path &outPath);
};

} // namespace S67
