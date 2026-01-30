#include "AssetProcessor.h"
#include "Core/Logger.h"
#include "Renderer/Mesh.h"
#include "stb_image.h"
#include <fstream>
#include <ios>
#include <nlohmann/json.hpp>

namespace S67 {

// --- TextureProcessor ---

struct TextureBinaryHeader {
  uint32_t Width;
  uint32_t Height;
  uint32_t Channels;
};

bool TextureProcessor::Process(const std::filesystem::path &inputPath,
                               ProcessedAsset &outAsset) {
  int width, height, channels;
  stbi_set_flip_vertically_on_load(1);
  unsigned char *data =
      stbi_load(inputPath.string().c_str(), &width, &height, &channels, 4);

  if (!data) {
    S67_CORE_ERROR("TextureProcessor: Failed to load {0}", inputPath.string());
    return false;
  }

  TextureBinaryHeader header;
  header.Width = (uint32_t)width;
  header.Height = (uint32_t)height;
  header.Channels = 4; // We force 4 channels

  size_t dataSize = width * height * 4;
  outAsset.Data.resize(sizeof(TextureBinaryHeader) + dataSize);
  memcpy(outAsset.Data.data(), &header, sizeof(TextureBinaryHeader));
  memcpy(outAsset.Data.data() + sizeof(TextureBinaryHeader), data, dataSize);

  outAsset.Name = inputPath.filename().string();
  outAsset.Type = "Texture";

  stbi_image_free(data);
  return true;
}

// --- MeshProcessor ---

struct MeshBinaryHeader {
  uint32_t VertexCount;
  uint32_t IndexCount;
};

bool MeshProcessor::Process(const std::filesystem::path &inputPath,
                            ProcessedAsset &outAsset) {
  // For now, we'll just store the raw OBJ/STL data or a simplified binary
  // version. Implementing a full binary mesh format requires extracting data
  // from MeshLoader logic. Let's do a simple binary dump of vertices and
  // indices if we can.

  // Actually, for this implementation, let's keep it simple and just bundle the
  // raw file but mark it as processed. A real engine would do vertex
  // optimization here.

  std::ifstream file(inputPath, std::ios::binary | std::ios::ate);
  if (!file.is_open())
    return false;

  size_t size = file.tellg();
  file.seekg(0, std::ios::beg);

  outAsset.Data.resize(size);
  file.read((char *)outAsset.Data.data(), size);

  outAsset.Name = inputPath.filename().string();
  outAsset.Type = "Mesh";

  return true;
}

// --- ShaderProcessor ---

bool ShaderProcessor::Process(const std::filesystem::path &inputPath,
                              ProcessedAsset &outAsset) {
  std::ifstream file(inputPath, std::ios::binary | std::ios::ate);
  if (!file.is_open())
    return false;

  size_t size = file.tellg();
  file.seekg(0, std::ios::beg);

  outAsset.Data.resize(size);
  file.read((char *)outAsset.Data.data(), size);

  outAsset.Name = inputPath.filename().string();
  outAsset.Type = "Shader";

  return true;
}

// --- LevelProcessor ---

bool LevelProcessor::Process(const std::filesystem::path &inputPath,
                             ProcessedAsset &outAsset) {
  // Just bundle the JSON for now, maybe minify it.
  std::ifstream file(inputPath);
  if (!file.is_open())
    return false;

  nlohmann::json data;
  file >> data;

  std::string dumped = data.dump(); // Minified
  outAsset.Data.assign(dumped.begin(), dumped.end());

  outAsset.Name = inputPath.filename().string();
  outAsset.Type = "Level";

  return true;
}

} // namespace S67
