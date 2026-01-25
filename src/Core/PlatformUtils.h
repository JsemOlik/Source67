#pragma once

#include <string>

namespace S67 {

    class FileDialogs {
    public:
        // Returns empty string if cancelled
        static std::string OpenFile(const char* filter, const char* extension);
        static std::string SaveFile(const char* filter, const char* defaultName, const char* extension);
        static std::string OpenFolder();
        static void OpenExplorer(const std::string& path);
        static void OpenExternally(const std::string& path);
    };

}
