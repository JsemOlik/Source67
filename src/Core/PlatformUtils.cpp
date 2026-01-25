#include "PlatformUtils.h"
#include <cstdio>
#include <string>

namespace S67 {

    std::string FileDialogs::OpenFile(const char* filter, const char* extension) {
        char buffer[1024];
        std::string result = "";
        
        // Command to open Finder and choose a file with specific extension
        std::string cmd = "osascript -e 'POSIX path of (choose file of type {\"" + std::string(extension) + "\"} with prompt \"Select a Source67 File\")' 2>/dev/null";
        
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) return "";
        
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            result += buffer;
        }
        pclose(pipe);
        
        if (!result.empty() && result.back() == '\n') result.pop_back();
        return result;
    }

    std::string FileDialogs::SaveFile(const char* filter, const char* defaultName, const char* extension) {
        char buffer[1024];
        std::string result = "";
        
        // Command to open Finder save dialog
        std::string cmd = "osascript -e 'POSIX path of (choose file name default name \"" + std::string(defaultName) + "." + std::string(extension) + "\" with prompt \"Save Source67 File\")' 2>/dev/null";
        
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) return "";
        
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            result += buffer;
        }
        pclose(pipe);
        
        if (!result.empty() && result.back() == '\n') result.pop_back();
        
        // Ensure extension
        std::string extStr = "." + std::string(extension);
        if (!result.empty() && result.find(extStr) == std::string::npos) {
            result += extStr;
        }
        
        return result;
    }

    std::string FileDialogs::OpenFolder() {
        char buffer[1024];
        std::string result = "";
        
        std::string cmd = "osascript -e 'POSIX path of (choose folder with prompt \"Select Project Root Folder\")' 2>/dev/null";
        
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) return "";
        
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            result += buffer;
        }
        pclose(pipe);
        
        if (!result.empty() && result.back() == '\n') result.pop_back();
        return result;
    }

    void FileDialogs::OpenExplorer(const std::string& path) {
        std::string cmd = "open -R \"" + path + "\"";
        system(cmd.c_str());
    }

    void FileDialogs::OpenExternally(const std::string& path) {
        std::string cmd = "open \"" + path + "\"";
        system(cmd.c_str());
    }

}
