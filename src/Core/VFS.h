#pragma once

#include "Base.h"
#include <filesystem>
#include <memory>
#include <string>
#include <vector>


namespace S67 {

class PakReader;

struct VFSFile {
  std::vector<uint8_t> Data;
  bool Success = false;
};

class VFS {
public:
  static void Init();
  static void Shutdown();

  // Mount a physical directory or a .pak file
  static void Mount(const std::string &path,
                    const std::string &virtualPath = "");

  static VFSFile Read(const std::string &path);
  static bool Exists(const std::string &path);

  static std::string GetPhysicalPath(const std::string &path);

private:
  struct MountPoint {
    std::string PhysicalPath;
    std::string VirtualPath;
    bool IsPak = false;
    std::unique_ptr<PakReader> Reader;
  };

  static std::vector<MountPoint> s_MountPoints;
};

} // namespace S67
