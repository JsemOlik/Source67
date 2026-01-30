#include "PlatformUtils.h"
#include "Logger.h"

#ifdef __APPLE__
#include <Cocoa/Cocoa.h>
#include <filesystem>

namespace S67 {

    void SetMacDockIcon(const std::string& path) {
        @autoreleasepool {
          std::filesystem::path absPath = std::filesystem::absolute(path);
          NSString *nsPath =
              [NSString stringWithUTF8String:absPath.string().c_str()];
          NSImage *image = [[NSImage alloc] initWithContentsOfFile:nsPath];
          if (image) {
            [NSApp setApplicationIconImage:image];
            S67_CORE_INFO("Applied macOS Dock icon from {0}", absPath.string());
          } else {
            S67_CORE_ERROR("Failed to load macOS Dock icon from {0}",
                           absPath.string());
          }
        }
    }

}
#endif
