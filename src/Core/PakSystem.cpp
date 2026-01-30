#include "PakSystem.h"
#include <algorithm>
#include <filesystem>
#include <iostream>

namespace S67 {

PakWriter::PakWriter(const std::string &outputPath)
    : m_OutputPath(outputPath) {}
PakWriter::~PakWriter() {}

void PakWriter::AddFile(const std::string &path, AssetType type,
                        bool compress) {
  if (!std::filesystem::exists(path))
    return;

  PakEntry entry;
  entry.path =
      std::filesystem::relative(path, std::filesystem::current_path()).string();
  std::replace(entry.path.begin(), entry.path.end(), '\\', '/');
  entry.type = type;
  entry.size = std::filesystem::file_size(path);
  entry.compressedSize = entry.size; // No compression for now
  entry.compressed = false;

  m_PendingFiles.push_back({path, entry});
}

bool PakWriter::Write() {
  std::ofstream file(m_OutputPath, std::ios::binary);
  if (!file)
    return false;

  PakHeader header;
  header.entryCount = (uint32_t)m_PendingFiles.size();

  // Write placeholder header
  file.write((char *)&header, sizeof(PakHeader));

  // Write data and update offsets
  for (auto &pf : m_PendingFiles) {
    pf.entry.offset = file.tellp();
    std::ifstream src(pf.fullPath, std::ios::binary);
    file << src.rdbuf();
    m_Entries.push_back(pf.entry);
  }

  // Write index
  header.indexOffset = file.tellp();
  for (const auto &entry : m_Entries) {
    uint32_t pathLen = (uint32_t)entry.path.length();
    file.write((char *)&pathLen, sizeof(uint32_t));
    file.write(entry.path.c_str(), pathLen);
    file.write((char *)&entry.type, sizeof(AssetType));
    file.write((char *)&entry.offset, sizeof(uint64_t));
    file.write((char *)&entry.size, sizeof(uint64_t));
    file.write((char *)&entry.compressedSize, sizeof(uint64_t));
    file.write((char *)&entry.compressed, sizeof(bool));
  }

  // Rewrite header with correct index offset
  file.seekp(0);
  file.write((char *)&header, sizeof(PakHeader));

  return true;
}

PakReader::PakReader(const std::string &path) : m_Path(path) {}
PakReader::~PakReader() {
  if (m_File.is_open())
    m_File.close();
}

PakReader *PakReader::Load(const std::string &path) {
  PakReader *reader = new PakReader(path);
  if (reader->Initialize())
    return reader;
  delete reader;
  return nullptr;
}

bool PakReader::Initialize() {
  m_File.open(m_Path, std::ios::binary);
  if (!m_File)
    return false;

  m_File.read((char *)&m_Header, sizeof(PakHeader));
  if (std::string(m_Header.magic, 4) != "AP67")
    return false;

  m_File.seekg(m_Header.indexOffset);
  for (uint32_t i = 0; i < m_Header.entryCount; ++i) {
    uint32_t pathLen;
    m_File.read((char *)&pathLen, sizeof(uint32_t));
    std::string path(pathLen, ' ');
    m_File.read(&path[0], pathLen);

    PakEntry entry;
    entry.path = path;
    m_File.read((char *)&entry.type, sizeof(AssetType));
    m_File.read((char *)&entry.offset, sizeof(uint64_t));
    m_File.read((char *)&entry.size, sizeof(uint64_t));
    m_File.read((char *)&entry.compressedSize, sizeof(uint64_t));
    m_File.read((char *)&entry.compressed, sizeof(bool));

    m_Index[path] = entry;
    m_Entries.push_back(entry);
  }

  return true;
}

bool PakReader::HasFile(const std::string &path) const {
  return m_Index.find(path) != m_Index.end();
}

std::vector<uint8_t> PakReader::ReadFile(const std::string &path) {
  auto it = m_Index.find(path);
  if (it == m_Index.end())
    return {};

  const auto &entry = it->second;
  std::vector<uint8_t> data(entry.size);
  m_File.seekg(entry.offset);
  m_File.read((char *)data.data(), entry.size);
  return data;
}

} // namespace S67
