#pragma once

#include <filesystem>
#include <string>
#include <vector>


namespace S67 {

struct ProcessedAsset {
  std::string Name;
  std::vector<uint8_t> Data;
  std::string Type; // "Texture", "Mesh", "Shader", "Level"
};

class AssetProcessor {
public:
  virtual ~AssetProcessor() = default;
  virtual bool Process(const std::filesystem::path &inputPath,
                       ProcessedAsset &outAsset) = 0;
};

class TextureProcessor : public AssetProcessor {
public:
  virtual bool Process(const std::filesystem::path &inputPath,
                       ProcessedAsset &outAsset) override;
};

class MeshProcessor : public AssetProcessor {
public:
  virtual bool Process(const std::filesystem::path &inputPath,
                       ProcessedAsset &outAsset) override;
};

class ShaderProcessor : public AssetProcessor {
public:
  virtual bool Process(const std::filesystem::path &inputPath,
                       ProcessedAsset &outAsset) override;
};

class LevelProcessor : public AssetProcessor {
public:
  virtual bool Process(const std::filesystem::path &inputPath,
                       ProcessedAsset &outAsset) override;
};

} // namespace S67
