#include "Console.h"
#include "Core/Logger.h"

#include <fstream>
#include <iostream>
#include <sstream>


namespace S67 {

std::unordered_map<std::string, ConCommandBase *> Console::s_Registry;

void Console::Init() {
  S67_CORE_INFO("Console System Initializing...");

  // Walk the static linked list and register everything
  ConCommandBase *pCur = ConCommandBase::s_ConCommandBaseHead;
  while (pCur) {
    Register(pCur);
    pCur = pCur->m_Next;
  }

  S67_CORE_INFO("Console registered {0} commands/variables.",
                s_Registry.size());

  // Load config
  LoadConfig("config.cfg");
}

void Console::Shutdown() {
  // Save config
  SaveConfig("config.cfg");
  s_Registry.clear();
}

void Console::Register(ConCommandBase *base) {
  if (!base || !base->GetName())
    return;

  std::string name = base->GetName();
  // Case insensitive? Source usually is insensitive.
  // For now, strict case or lower case everything.
  // Let's enforce lower case names for keys in map for lookup
  // std::transform(name.begin(), name.end(), name.begin(), ::tolower);

  if (s_Registry.find(name) != s_Registry.end()) {
    S67_CORE_WARN("Console: Duplicate command/var name '{0}'", name);
    return;
  }

  s_Registry[name] = base;
}

void Console::Unregister(ConCommandBase *base) {
  // ...
}

ConVar *Console::FindVar(const char *name) {
  auto it = s_Registry.find(name);
  if (it != s_Registry.end()) {
    return dynamic_cast<ConVar *>(it->second);
  }
  return nullptr;
}

ConCommand *Console::FindCommand(const char *name) {
  auto it = s_Registry.find(name);
  if (it != s_Registry.end()) {
    // We can't safely dynamic_cast unless we enable RTTI or add virtual helper
    // We added IsCommand() but that was virtual in header.
    // Let's use flexible approach:
    if (it->second->IsCommand())
      return (ConCommand *)
          it->second; // unsafe cast if we aren't sure, but IsCommand says yes
  }
  return nullptr;
}

ConCommandBase *Console::FindBase(const char *name) {
  auto it = s_Registry.find(name);
  if (it != s_Registry.end()) {
    return it->second;
  }
  return nullptr;
}

void Console::ExecuteCommand(const std::string &commandString) {
  if (commandString.empty())
    return;

  S67_CORE_TRACE("CMD: {0}", commandString);

  // Simple tokenizer handling quotes
  std::vector<std::string> args = Tokenize(commandString);
  if (args.empty())
    return;

  std::string commandName = args[0];
  // Pop valid args
  // Actually CCommand expects all args including name

  ConCommandBase *base = FindBase(commandName.c_str());
  if (base) {
    // Check flags (FCVAR_CHEAT, etc) logic here if we had a cheat system active
    // if (base->IsFlagSet(FCVAR_CHEAT) && !sv_cheats->GetBool()) ...

    if (base->IsCommand()) {
      ConCommand *cmd = (ConCommand *)base;
      CCommand ccmd(args.size(), args);
      cmd->Dispatch(ccmd);
    } else {
      // It's a variable
      ConVar *var = (ConVar *)base;
      if (args.size() > 1) {
        // Setting value
        // todo: concatenate remaining args if string?
        // For now, just take second arg
        var->SetValue(args[1].c_str());
      } else {
        // Print value
        S67_CORE_INFO("{0} = \"{1}\" (def: \"{2}\")", var->GetName(),
                      var->GetString(), var->GetDefault());
        if (var->GetHelpString()) {
          S67_CORE_INFO(" - {0}", var->GetHelpString());
        }
      }
    }
  } else {
    S67_CORE_WARN("Unknown command: {0}", commandName);
  }
}

std::vector<std::string> Console::Tokenize(const std::string &text) {
  std::vector<std::string> tokens;
  std::string currentToken;
  bool inQuotes = false;

  for (size_t i = 0; i < text.length(); ++i) {
    char c = text[i];

    if (c == '\"') {
      inQuotes = !inQuotes;
      continue;
    }

    if (isspace(c) && !inQuotes) {
      if (!currentToken.empty()) {
        tokens.push_back(currentToken);
        currentToken.clear();
      }
    } else {
      currentToken += c;
    }
  }

  if (!currentToken.empty()) {
    tokens.push_back(currentToken);
  }

  return tokens;
}

void Console::SaveConfig(const std::string &filepath) {
  std::ofstream out(filepath);
  if (!out.is_open())
    return;

  out << "// Source67 Config" << std::endl;

  for (auto &pair : s_Registry) {
    ConCommandBase *base = pair.second;
    if (base && !base->IsCommand() && base->IsFlagSet(FCVAR_ARCHIVE)) {
      ConVar *var = (ConVar *)base;
      // Quote string if it has spaces?
      // Simple serialization
      out << var->GetName() << " \"" << var->GetString() << "\"" << std::endl;
    }
  }

  out.close();
  S67_CORE_INFO("Saved config to {0}", filepath);
}

void Console::LoadConfig(const std::string &filepath) {
  std::ifstream in(filepath);
  if (!in.is_open())
    return;

  std::string line;
  while (std::getline(in, line)) {
    // Trim whitespace
    // Skip comments
    // Execute as command
    if (line.empty() || line.rfind("//", 0) == 0)
      continue;
    ExecuteCommand(line);
  }

  in.close();
  S67_CORE_INFO("Loaded config from {0}", filepath);
}

} // namespace S67
