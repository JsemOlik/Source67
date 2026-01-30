#include "PakSystem.h"
#include "Core/Logger.h"
#include <filesystem>
#include <fstream>

namespace S67 {

// --- PakWriter ---

PakWriter::PakWriter(const std::string &filepath) : m_FilePath(filepath) {}
PakWriter::~PakWriter() {}

void PakWriter::AddFile(const std::string &name, const void *data,
                        uint32_t size) {
  std::vector<uint8_t> buffer((uint8_t *)data, (uint8_t *)data + size);
  m_Files[name] = std::move(buffer);
}

void PakWriter::AddFile(const std::string &name,
                        const std::string &sourcePath) {
  std::ifstream file(sourcePath, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    S67_CORE_ERROR("PakWriter: Failed to open source file {0}", sourcePath);
    return;
  }

  size_t size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<uint8_t> buffer(size);
  file.read((char *)buffer.data(), size);
  m_Files[name] = std::move(buffer);
}

bool PakWriter::Write() {
  std::ofstream fout(m_FilePath, std::ios::binary);
  if (!fout.is_open()) {
    S67_CORE_ERROR("PakWriter: Failed to create pak file {0}", m_FilePath);
    return false;
  }

  PakHeader header;
  header.FileCount = (uint32_t)m_Files.size();

  // Skip header for now
  fout.seekp(sizeof(PakHeader));

  std::vector<PakEntry> entries;
  for (auto &[name, data] : m_Files) {
    PakEntry entry;
    entry.Name = name;
    entry.Offset = (uint32_t)fout.tellp();
    entry.Size = (uint32_t)data.size();
    entry.CompressedSize = 0; // No compression yet

    fout.write((char *)data.data(), data.size());
    entries.push_back(entry);
  }

  header.TOCOffset = (uint32_t)fout.tellp();

  // Write TOC
  for (const auto &entry : entries) {
    uint32_t nameLen = (uint32_t)entry.Name.length();
    fout.write((char *)&nameLen, sizeof(uint32_t));
    fout.write(entry.Name.c_str(), nameLen);
    fout.write((char *)&entry.Offset, sizeof(uint32_t));
    fout.write((char *)&entry.Size, sizeof(uint32_t));
    fout.write((char *)&entry.CompressedSize, sizeof(uint32_t));
  }

  // Go back and write header
  fout.seekp(0);
  fout.write((char *)&header, sizeof(PakHeader));

  fout.close();
  S67_CORE_INFO("PakWriter: Successfully wrote {0} with {1} files", m_FilePath,
                header.FileCount);
  return true;
}

// --- PakReader ---

PakReader::PakReader(const std::string &filepath) : m_FilePath(filepath) {}
PakReader::~PakReader() {
  if (m_FileStream.is_open())
    m_FileStream.close();
}

bool PakReader::Open() {
  m_FileStream.open(m_FilePath, std::ios::binary);
  if (!m_FileStream.is_open()) {
    S67_CORE_ERROR("PakReader: Failed to open pak file {0}", m_FilePath);
    return false;
  }

  m_FileStream.read((char *)&m_Header, sizeof(PakHeader));
  if (m_Header.Magic[0] != 'S' || m_Header.Magic[1] != '6' ||
      m_Header.Magic[2] != '7' || m_Header.Magic[3] != 'P') {
    S67_CORE_ERROR("PakReader: Invalid magic in {0}", m_FilePath);
    return false;
  }

  m_FileStream.seekg(m_Header.TOCOffset);
  for (uint32_t i = 0; i < m_Header.FileCount; ++i) {
    PakEntry entry;
    uint32_t nameLen;
    m_FileStream.read((char *)&nameLen, sizeof(uint32_t));

    char *nameBuf = new char[nameLen + 1];
    m_FileStream.read(nameBuf, nameLen);
    nameBuf[nameLen] = '\0';
    entry.Name = nameBuf;
    delete[] nameBuf;

    m_FileStream.read((char *)&entry.Offset, sizeof(uint32_t));
    m_FileStream.read((char *)&entry.Size, sizeof(uint32_t));
    m_FileStream.read((char *)&entry.CompressedSize, sizeof(uint32_t));

    m_Entries[entry.Name] = entry;
  }

  m_IsOpen = true;
  return true;
}

bool PakReader::ExtractFile(const std::string &name,
                            std::vector<uint8_t> &outBuffer) {
  if (!m_IsOpen)
    return false;

  auto it = m_Entries.find(name);
  if (it == m_Entries.end())
    return false;

  const auto &entry = it->second;
  outBuffer.resize(entry.Size);

  m_FileStream.seekg(entry.Offset);
  m_FileStream.read((char *)outBuffer.data(), entry.Size);

  return true;
}

bool PakReader::HasFile(const std::string &name) const {
  return m_Entries.find(name) != m_Entries.end();
}

} // namespace S67
