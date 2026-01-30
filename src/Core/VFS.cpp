#include "VFS.h"
#include "Logger.h"
#include "PakSystem.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace S67 {

std::vector<VFS::MountPoint> VFS::s_MountPoints;

void VFS::Init() { S67_CORE_INFO("VFS Initialized"); }

void VFS::Shutdown() {
  s_MountPoints.clear();
  S67_CORE_INFO("VFS Shutdown");
}

void VFS::Mount(const std::string &path, const std::string &virtualPath) {
  MountPoint mp;
  mp.PhysicalPath = path;
  mp.VirtualPath = virtualPath;
  mp.IsPak = std::filesystem::path(path).extension() == ".pak";

  if (mp.IsPak) {
    mp.Reader = std::make_unique<PakReader>(path);
    if (!mp.Reader->IsOpen()) {
      S67_CORE_ERROR("VFS: Failed to open Pak file '{0}'", path);
      return;
    }
  }

  s_MountPoints.push_back(std::move(mp));
  S67_CORE_INFO("VFS: Mounted '{0}' to '{1}' ({2})", path, virtualPath,
                mp.IsPak ? "PAK" : "DIR");
}

VFSFile VFS::Read(const std::string &path) {
  // Try to find in mount points
  for (const auto &mp : s_MountPoints) {
    if (path.find(mp.VirtualPath) == 0) {
      std::string relative = path.substr(mp.VirtualPath.length());
      if (relative.empty() || relative[0] == '/' || relative[0] == '\\') {
        // Strip leading slash
        if (!relative.empty() && (relative[0] == '/' || relative[0] == '\\'))
          relative = relative.substr(1);

        if (mp.IsPak) {
          VFSFile result;
          if (mp.Reader->GetFileData(relative, result.Data)) {
            result.Success = true;
            return result;
          }
        } else {
          std::filesystem::path full =
              std::filesystem::path(mp.PhysicalPath) / relative;
          if (std::filesystem::exists(full)) {
            std::ifstream file(full, std::ios::binary | std::ios::ate);
            if (file.is_open()) {
              std::streamsize size = file.tellg();
              file.seekg(0, std::ios::beg);
              VFSFile result;
              result.Data.resize(size);
              if (file.read((char *)result.Data.data(), size)) {
                result.Success = true;
                return result;
              }
            }
          }
        }
      }
    }
  }

  // Fallback: Check physical path directly
  if (std::filesystem::exists(path) && !std::filesystem::is_directory(path)) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (file.is_open()) {
      std::streamsize size = file.tellg();
      file.seekg(0, std::ios::beg);
      VFSFile result;
      result.Data.resize(size);
      if (file.read((char *)result.Data.data(), size)) {
        result.Success = true;
        return result;
      }
    }
  }

  S67_CORE_WARN("VFS: Could not find file '{0}'", path);
  return {{}, false};
}

bool VFS::Exists(const std::string &path) {
  for (const auto &mp : s_MountPoints) {
    if (path.find(mp.VirtualPath) == 0) {
      std::string relative = path.substr(mp.VirtualPath.length());
      if (relative.empty() || relative[0] == '/' || relative[0] == '\\') {
        if (!relative.empty() && (relative[0] == '/' || relative[0] == '\\'))
          relative = relative.substr(1);

        if (mp.IsPak) {
          std::vector<uint8_t> dummy;
          if (mp.Reader->GetFileData(relative, dummy))
            return true;
        } else {
          std::filesystem::path full =
              std::filesystem::path(mp.PhysicalPath) / relative;
          if (std::filesystem::exists(full))
            return true;
        }
      }
    }
  }
  return std::filesystem::exists(path) && !std::filesystem::is_directory(path);
}

std::string VFS::GetPhysicalPath(const std::string &path) {
  if (std::filesystem::path(path).is_absolute() &&
      std::filesystem::exists(path))
    return path;

  for (const auto &mp : s_MountPoints) {
    if (path.find(mp.VirtualPath) == 0) {
      std::string relative = path.substr(mp.VirtualPath.length());
      if (relative.empty() || relative[0] == '/' || relative[0] == '\\') {
        if (mp.IsPak)
          continue; // Cannot return a physical path for a pak file entry

        std::filesystem::path full =
            std::filesystem::path(mp.PhysicalPath) / relative;
        if (std::filesystem::exists(full)) {
          return full.string();
        }
      }
    }
  }

  if (std::filesystem::exists(path))
    return std::filesystem::absolute(path).string();

  return "";
}

} // namespace S67
