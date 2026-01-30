#pragma once

#include <cstdint>
#include <fstream>
#include <map>
#include <string>
#include <vector>


namespace S67 {

struct PakHeader {
  char Magic[4] = {'S', '6', '7', 'P'};
  uint32_t Version = 1;
  uint32_t FileCount = 0;
  uint32_t TOCOffset = 0; // Offset to the Table of Contents
};

struct PakEntry {
  std::string Name;
  uint32_t Offset;
  uint32_t Size;
  uint32_t CompressedSize; // 0 if not compressed
};

class PakWriter {
public:
  PakWriter(const std::string &filepath);
  ~PakWriter();

  void AddFile(const std::string &name, const void *data, uint32_t size);
  void AddFile(const std::string &name, const std::string &sourcePath);
  bool Write();

private:
  std::string m_FilePath;
  std::map<std::string, std::vector<uint8_t>> m_Files;
};

class PakReader {
public:
  PakReader(const std::string &filepath);
  ~PakReader();

  bool Open();
  bool ExtractFile(const std::string &name, std::vector<uint8_t> &outBuffer);
  bool HasFile(const std::string &name) const;

  const std::map<std::string, PakEntry> &GetEntries() const {
    return m_Entries;
  }

private:
  std::string m_FilePath;
  std::ifstream m_FileStream;
  PakHeader m_Header;
  std::map<std::string, PakEntry> m_Entries;
  bool m_IsOpen = false;
};

} // namespace S67
