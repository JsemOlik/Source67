#pragma once
#include "PakSystem.h"
#include <filesystem>
#include <memory>
#include <string>
#include <vector>


namespace S67 {

class VFS {
public:
  static void Init();
  static void Shutdown();

  static void Mount(const std::string &path);
  static void Unmount(const std::string &path);

  static bool Exists(const std::string &path);
  static std::vector<uint8_t> ReadFile(const std::string &path);
  static std::string ReadFileToString(const std::string &path);

  // Resolves a virtual path to a physical path if possible (only for Editor)
  static std::filesystem::path ResolvePhysicalPath(const std::string &path);

private:
  struct MountPoint {
    std::string path;
    std::unique_ptr<PakReader> reader; // null if it's a raw directory
  };

  static std::vector<MountPoint> s_MountPoints;
};

} // namespace S67
