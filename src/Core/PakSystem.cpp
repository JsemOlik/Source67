#include "PakSystem.h"
#include "Logger.h"
#include <filesystem>
#include <fstream>


namespace S67 {

PakWriter::PakWriter(const std::string &filepath) : m_FilePath(filepath) {}

void PakWriter::AddFile(const std::string &name,
                        const std::vector<uint8_t> &data) {
  m_Files[name] = data;
}

bool PakWriter::Write() {
  std::ofstream file(m_FilePath, std::ios::binary);
  if (!file.is_open())
    return false;

  PakHeader header;
  header.EntryCount = (uint32_t)m_Files.size();

  // Write placeholder header
  file.write((char *)&header, sizeof(PakHeader));

  std::vector<PakEntry> entries;
  for (auto const &[name, data] : m_Files) {
    PakEntry entry;
    entry.Name = name;
    entry.Offset = file.tellp();
    entry.Size = data.size();
    entry.CompressedSize = data.size();

    file.write((char *)data.data(), data.size());
    entries.push_back(entry);
  }

  header.IndexOffset = file.tellp();

  // Write Index
  for (const auto &entry : entries) {
    uint32_t nameLen = (uint32_t)entry.Name.length();
    file.write((char *)&nameLen, sizeof(uint32_t));
    file.write(entry.Name.c_str(), nameLen);
    file.write((char *)&entry.Offset, sizeof(uint64_t));
    file.write((char *)&entry.Size, sizeof(uint64_t));
    file.write((char *)&entry.CompressedSize, sizeof(uint64_t));
  }

  // GO BACK and write final header
  file.seekp(0, std::ios::beg);
  file.write((char *)&header, sizeof(PakHeader));

  file.close();
  return true;
}

PakReader::PakReader(const std::string &filepath) {
  m_File = fopen(filepath.c_str(), "rb");
  if (!m_File)
    return;

  PakHeader header;
  if (fread(&header, sizeof(PakHeader), 1, m_File) != 1) {
    fclose(m_File);
    m_File = nullptr;
    return;
  }

  if (header.Magic[0] != 'S' || header.Magic[1] != '6' ||
      header.Magic[2] != '7' || header.Magic[3] != 'P') {
    fclose(m_File);
    m_File = nullptr;
    return;
  }

  fseek(m_File, (long)header.IndexOffset, SEEK_SET);

  for (uint32_t i = 0; i < header.EntryCount; i++) {
    PakEntry entry;
    uint32_t nameLen;
    fread(&nameLen, sizeof(uint32_t), 1, m_File);

    char *nameBuffer = new char[nameLen + 1];
    fread(nameBuffer, nameLen, 1, m_File);
    nameBuffer[nameLen] = '\0';
    entry.Name = nameBuffer;
    delete[] nameBuffer;

    fread(&entry.Offset, sizeof(uint64_t), 1, m_File);
    fread(&entry.Size, sizeof(uint64_t), 1, m_File);
    fread(&entry.CompressedSize, sizeof(uint64_t), 1, m_File);

    m_Entries[entry.Name] = entry;
  }
}

std::vector<std::string> PakReader::GetFileList() const {
  std::vector<std::string> files;
  for (auto const &[name, entry] : m_Entries) {
    files.push_back(name);
  }
  return files;
}

bool PakReader::GetFileData(const std::string &name,
                            std::vector<uint8_t> &outData) {
  if (m_Entries.find(name) == m_Entries.end())
    return false;

  const auto &entry = m_Entries[name];
  outData.resize(entry.Size);
  fseek(m_File, (long)entry.Offset, SEEK_SET);
  return fread(outData.data(), entry.Size, 1, m_File) == 1;
}

} // namespace S67
