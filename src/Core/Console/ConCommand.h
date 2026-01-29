#pragma once

#include "ConVar.h"
#include <string>
#include <vector>


namespace S67 {

// -------------------------------------------------------------------------
// CCommand
// Encapsulates valid command line arguments
// -------------------------------------------------------------------------
class CCommand {
public:
  CCommand();
  CCommand(int argc, const char **argv);
  CCommand(int argc, const std::vector<std::string> &args);

  int ArgC() const;
  const char *Arg(int nIndex) const;
  const char *operator[](int nIndex) const;

  // Helper to get the full command string (reconstructed)
  // const char* GetCommandString() const;

private:
  std::vector<std::string> m_Args;
};

// Callback type
using FnCommandCallback = std::function<void(const CCommand &)>;

// -------------------------------------------------------------------------
// ConCommand
// Console Command
// -------------------------------------------------------------------------
class ConCommand : public ConCommandBase {
public:
  ConCommand(const char *name, FnCommandCallback callback,
             const char *helpString = nullptr, int flags = 0);

  virtual ~ConCommand() {}

  void Dispatch(const CCommand &command);

  // This makes IsCommand() return true, hiding the Base declaration if strictly
  // virtual (it wasn't pure) Ideally we'd modify Base to have virtual IsCommand
  bool IsCommand() const { return true; }

private:
  FnCommandCallback m_Callback;
};

} // namespace S67
