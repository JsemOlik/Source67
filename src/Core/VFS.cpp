#include "VFS.h"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace S67 {

std::vector<VFS::MountPoint> VFS::s_MountPoints;

void VFS::Init() {
  // Default mount points
#ifdef S67_EDITOR
  Mount("assets"); // Priority 1: Raw assets (if in editor)
#endif
  Mount("GameAssets.apak"); // Priority 2: Packed assets
}

void VFS::Shutdown() { s_MountPoints.clear(); }

void VFS::Mount(const std::string &path) {
  if (std::filesystem::is_directory(path)) {
    s_MountPoints.push_back({path, nullptr});
  } else if (std::filesystem::exists(path) &&
             std::filesystem::path(path).extension() == ".apak") {
    auto reader = PakReader::Load(path);
    if (reader) {
      s_MountPoints.push_back({path, std::unique_ptr<PakReader>(reader)});
    }
  }
}

bool VFS::Exists(const std::string &path) {
  for (const auto &mp : s_MountPoints) {
    if (mp.reader) {
      if (mp.reader->HasFile(path))
        return true;
    } else {
      if (std::filesystem::exists(std::filesystem::path(mp.path) / path))
        return true;
    }
  }
  return false;
}

std::vector<uint8_t> VFS::ReadFile(const std::string &path) {
  for (const auto &mp : s_MountPoints) {
    if (mp.reader) {
      if (mp.reader->HasFile(path))
        return mp.reader->ReadFile(path);
    } else {
      auto physicalPath = std::filesystem::path(mp.path) / path;
      if (std::filesystem::exists(physicalPath)) {
        std::ifstream file(physicalPath, std::ios::binary | std::ios::ate);
        if (!file)
          continue;
        size_t size = file.tellg();
        std::vector<uint8_t> data(size);
        file.seekg(0);
        file.read((char *)data.data(), size);
        return data;
      }
    }
  }
  return {};
}

std::string VFS::ReadFileToString(const std::string &path) {
  auto data = ReadFile(path);
  return std::string(data.begin(), data.end());
}

std::filesystem::path VFS::ResolvePhysicalPath(const std::string &path) {
  for (const auto &mp : s_MountPoints) {
    if (!mp.reader) {
      auto physicalPath = std::filesystem::path(mp.path) / path;
      if (std::filesystem::exists(physicalPath))
        return physicalPath;
    }
  }
  return std::filesystem::path();
}

} // namespace S67
