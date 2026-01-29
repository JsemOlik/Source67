#include "ConCommand.h"

namespace S67 {

// -------------------------------------------------------------------------
// CCommand
// -------------------------------------------------------------------------
CCommand::CCommand() {}

CCommand::CCommand(int argc, const char **argv) {
  for (int i = 0; i < argc; ++i) {
    m_Args.push_back(argv[i]);
  }
}

CCommand::CCommand(int argc, const std::vector<std::string> &args)
    : m_Args(args) {}

int CCommand::ArgC() const { return (int)m_Args.size(); }

const char *CCommand::Arg(int nIndex) const {
  if (nIndex < 0 || nIndex >= (int)m_Args.size())
    return "";
  return m_Args[nIndex].c_str();
}

const char *CCommand::operator[](int nIndex) const { return Arg(nIndex); }

// -------------------------------------------------------------------------
// ConCommand
// -------------------------------------------------------------------------

ConCommand::ConCommand(const char *name, FnCommandCallback callback,
                       const char *helpString, int flags)
    : ConCommandBase(name, helpString, flags), m_Callback(callback) {}

void ConCommand::Dispatch(const CCommand &command) {
  if (m_Callback) {
    m_Callback(command);
  }
}

} // namespace S67
