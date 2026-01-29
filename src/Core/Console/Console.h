#pragma once

#include "ConCommand.h"
#include "ConVar.h"
#include <string>
#include <unordered_map>
#include <vector>


namespace S67 {

class Console {
public:
  static void Init();
  static void Shutdown();

  static void ExecuteCommand(const std::string &commandString);

  static ConVar *FindVar(const char *name);
  static ConCommand *FindCommand(const char *name);
  static ConCommandBase *FindBase(const char *name);

  // Helper for auto-complete later
  static const std::unordered_map<std::string, ConCommandBase *> &
  GetRegistry() {
    return s_Registry;
  }

  // Persistence
  static void SaveConfig(const std::string &filepath);
  static void LoadConfig(const std::string &filepath);

private:
  static void Register(ConCommandBase *base);
  static void Unregister(ConCommandBase *base);

  // Parsing helpers
  static std::vector<std::string> Tokenize(const std::string &text);

private:
  static std::unordered_map<std::string, ConCommandBase *> s_Registry;
};

} // namespace S67
