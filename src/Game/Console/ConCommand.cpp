#include "ConCommand.h"
#include "Console.h"

namespace S67 {

ConCommand::ConCommand(const std::string &name, ConCommandCallback callback,
                       const std::string &helpString, int flags)
    : m_Name(name), m_Callback(callback), m_HelpString(helpString),
      m_Flags(flags) {
  Console::Get().RegisterCommand(this);
}

ConCommand::~ConCommand() { Console::Get().UnregisterCommand(this); }

void ConCommand::Execute(const ConCommandArgs &args) {
  if (m_Callback) {
    m_Callback(args);
  }
}

} // namespace S67
