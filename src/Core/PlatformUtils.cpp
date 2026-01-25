#include "PlatformUtils.h"
#include <cstdio>
#include <string>

namespace S67 {

    std::string FileDialogs::OpenFile(const char* filter) {
        char buffer[1024];
        std::string result = "";
        
        // Command to open Finder and choose a file with .s67 extension
        std::string cmd = "osascript -e 'POSIX path of (choose file of type {\"s67\"} with prompt \"Select a Source67 Level\")' 2>/dev/null";
        
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) return "";
        
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            result += buffer;
        }
        pclose(pipe);
        
        if (!result.empty() && result.back() == '\n') result.pop_back();
        return result;
    }

    std::string FileDialogs::SaveFile(const char* filter) {
        char buffer[1024];
        std::string result = "";
        
        // Command to open Finder save dialog
        std::string cmd = "osascript -e 'POSIX path of (choose file name default name \"level.s67\" with prompt \"Save Source67 Level\")' 2>/dev/null";
        
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) return "";
        
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            result += buffer;
        }
        pclose(pipe);
        
        if (!result.empty() && result.back() == '\n') result.pop_back();
        
        // Ensure extension
        if (!result.empty() && result.find(".s67") == std::string::npos) {
            result += ".s67";
        }
        
        return result;
    }

}
