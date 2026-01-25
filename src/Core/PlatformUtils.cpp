#include "PlatformUtils.h"
#include <cstdio>
#include <string>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
// clang-format off
#include <commdlg.h>
#include <shellapi.h>
#include <shlobj.h>
// clang-format on

#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#endif

namespace S67 {

#ifdef _WIN32
std::string FileDialogs::OpenFile(const char *filter, const char *extension) {
  OPENFILENAMEA ofn;
  CHAR szFile[260] = {0};
  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = NULL;
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile);
  ofn.lpstrFilter = filter;
  ofn.nFilterIndex = 1;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

  if (GetOpenFileNameA(&ofn) == TRUE) {
    return ofn.lpstrFile;
  }
  return "";
}

std::string FileDialogs::SaveFile(const char *filter, const char *defaultName,
                                  const char *extension) {
  OPENFILENAMEA ofn;
  CHAR szFile[260] = {0};
  if (defaultName) {
    strncpy_s(szFile, defaultName, sizeof(szFile) - 1);
    if (extension) {
      strncat_s(szFile, ".", sizeof(szFile) - strlen(szFile) - 1);
      strncat_s(szFile, extension, sizeof(szFile) - strlen(szFile) - 1);
    }
  }

  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = NULL;
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile);
  ofn.lpstrFilter = filter;
  ofn.nFilterIndex = 1;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

  if (GetSaveFileNameA(&ofn) == TRUE) {
    return ofn.lpstrFile;
  }
  return "";
}

std::string FileDialogs::OpenFolder() {
  CHAR szDir[MAX_PATH];
  BROWSEINFOA bi = {0};
  bi.lpszTitle = "Select Project Root Folder";
  bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
  LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
  if (pidl != NULL) {
    SHGetPathFromIDListA(pidl, szDir);
    CoTaskMemFree(pidl);
    return szDir;
  }
  return "";
}

void FileDialogs::OpenExplorer(const std::string &path) {
  ShellExecuteA(NULL, "explore", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void FileDialogs::OpenExternally(const std::string &path) {
  ShellExecuteA(NULL, "open", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

#elif defined(__APPLE__)

std::string FileDialogs::OpenFile(const char *filter, const char *extension) {
  char buffer[1024];
  std::string result = "";

  // Command to open Finder and choose a file with specific extension
  std::string cmd = "osascript -e 'POSIX path of (choose file of type {\"" +
                    std::string(extension) +
                    "\"} with prompt \"Select a Source67 File\")' 2>/dev/null";

  FILE *pipe = popen(cmd.c_str(), "r");
  if (!pipe)
    return "";

  while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
    result += buffer;
  }
  pclose(pipe);

  if (!result.empty() && result.back() == '\n')
    result.pop_back();
  return result;
}

std::string FileDialogs::SaveFile(const char *filter, const char *defaultName,
                                  const char *extension) {
  char buffer[1024];
  std::string result = "";

  // Command to open Finder save dialog
  std::string cmd =
      "osascript -e 'POSIX path of (choose file name default name \"" +
      std::string(defaultName) + "." + std::string(extension) +
      "\" with prompt \"Save Source67 File\")' 2>/dev/null";

  FILE *pipe = popen(cmd.c_str(), "r");
  if (!pipe)
    return "";

  while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
    result += buffer;
  }
  pclose(pipe);

  if (!result.empty() && result.back() == '\n')
    result.pop_back();

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

  std::string cmd = "osascript -e 'POSIX path of (choose folder with prompt "
                    "\"Select Project Root Folder\")' 2>/dev/null";

  FILE *pipe = popen(cmd.c_str(), "r");
  if (!pipe)
    return "";

  while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
    result += buffer;
  }
  pclose(pipe);

  if (!result.empty() && result.back() == '\n')
    result.pop_back();
  return result;
}

void FileDialogs::OpenExplorer(const std::string &path) {
  std::string cmd = "open -R \"" + path + "\"";
  system(cmd.c_str());
}

void FileDialogs::OpenExternally(const std::string &path) {
  std::string cmd = "open \"" + path + "\"";
  system(cmd.c_str());
}
#else
// Fallback for other platforms
std::string FileDialogs::OpenFile(const char *filter, const char *extension) {
  return "";
}
std::string FileDialogs::SaveFile(const char *filter, const char *defaultName,
                                  const char *extension) {
  return "";
}
std::string FileDialogs::OpenFolder() { return ""; }
void FileDialogs::OpenExplorer(const std::string &path) {}
void FileDialogs::OpenExternally(const std::string &path) {}
#endif

} // namespace S67
