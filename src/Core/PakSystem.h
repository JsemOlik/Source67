#pragma once
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>


namespace S67 {

enum class AssetType : uint32_t {
  ASSET_UNKNOWN = 0,
  ASSET_TEXTURE = 1,
  ASSET_MODEL = 2,
  ASSET_SCENE = 3,
  ASSET_SHADER = 4,
  ASSET_FONT = 5,
  ASSET_LUA_SCRIPT = 6,
  ASSET_CONFIG_JSON = 7,
  ASSET_AUDIO = 8,
  ASSET_ANIMATION = 9,
};

struct PakEntry {
  std::string path;
  AssetType type;
  uint64_t offset;
  uint64_t size;
  uint64_t compressedSize;
  bool compressed;
};

struct PakHeader {
  char magic[4] = {'A', 'P', '6', '7'};
  uint32_t version = 2;
  uint32_t entryCount = 0;
  uint64_t indexOffset = 0;
  uint32_t luaScriptCount = 0;
  uint32_t flags = 0;
};

class PakWriter {
public:
  PakWriter(const std::string &outputPath);
  ~PakWriter();

  void AddFile(const std::string &path, AssetType type, bool compress = true);
  bool Write();

private:
  std::string m_OutputPath;
  std::vector<PakEntry> m_Entries;
  struct PendingFile {
    std::string fullPath;
    PakEntry entry;
  };
  std::vector<PendingFile> m_PendingFiles;
};

class PakReader {
public:
  static PakReader *Load(const std::string &path);
  ~PakReader();

  bool HasFile(const std::string &path) const;
  std::vector<uint8_t> ReadFile(const std::string &path);

  const std::vector<PakEntry> &GetEntries() const { return m_Entries; }
  const PakHeader &GetHeader() const { return m_Header; }

private:
  PakReader(const std::string &path);
  bool Initialize();

  std::string m_Path;
  std::ifstream m_File;
  PakHeader m_Header;
  std::map<std::string, PakEntry> m_Index;
  std::vector<PakEntry> m_Entries;
};

} // namespace S67
