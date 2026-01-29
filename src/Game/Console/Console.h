#pragma once
#include "ConCommand.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace S67 {

class ConVar;

class Console {
public:
  static Console &Get();

  void RegisterConVar(ConVar *cvar);
  void UnregisterConVar(ConVar *cvar);
  ConVar *FindConVar(const std::string &name);

  void RegisterCommand(ConCommand *cmd);
  void UnregisterCommand(ConCommand *cmd);
  ConCommand *FindCommand(const std::string &name);

  void ExecuteCommand(const std::string &commandLine);

  void Save(const std::string &filename = "game.cfg");
  void Load(const std::string &filename = "game.cfg");

  // Helper for auto-completion or listing
  const std::unordered_map<std::string, ConVar *> &GetConVars() const {
    return m_ConVars;
  }
  const std::unordered_map<std::string, ConCommand *> &GetCommands() const {
    return m_Commands;
  }

  // History
  void AddLog(const std::string &message);
  const std::vector<std::string> &GetLogHistory() const { return m_LogHistory; }
  void ClearLog() { m_LogHistory.clear(); }

private:
  Console() = default;
  ~Console() = default;

  Console(const Console &) = delete;
  Console &operator=(const Console &) = delete;

  std::unordered_map<std::string, ConVar *> m_ConVars;
  std::unordered_map<std::string, ConCommand *> m_Commands;

  std::vector<std::string> m_LogHistory;
};

} // namespace S67
