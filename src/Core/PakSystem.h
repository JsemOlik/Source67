#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>


namespace S67 {

struct PakEntry {
  std::string Name;
  uint64_t Offset;
  uint64_t Size;
  uint64_t CompressedSize; // Placeholder for future compression
};

struct PakHeader {
  char Magic[4] = {'S', '6', '7', 'P'};
  uint32_t Version = 1;
  uint32_t EntryCount = 0;
  uint64_t IndexOffset = 0;
};

class PakWriter {
public:
  PakWriter(const std::string &filepath);
  void AddFile(const std::string &name, const std::vector<uint8_t> &data);
  bool Write();

private:
  std::string m_FilePath;
  std::map<std::string, std::vector<uint8_t>> m_Files;
};

class PakReader {
public:
  PakReader(const std::string &filepath);
  bool IsOpen() const { return m_File != nullptr; }

  std::vector<std::string> GetFileList() const;
  bool GetFileData(const std::string &name, std::vector<uint8_t> &outData);

private:
  struct InternalFile {
    FILE *m_File;
    std::map<std::string, PakEntry> m_Entries;
  };
  FILE *m_File = nullptr;
  std::map<std::string, PakEntry> m_Entries;
};

} // namespace S67
