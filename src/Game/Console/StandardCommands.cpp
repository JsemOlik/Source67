#include "ConVar.h"
#include "Console.h"
#include "Core/Logger.h"
#include <iomanip>
#include <sstream>

namespace S67 {

// Helper to register standard commands statically
struct StandardCommands {
  StandardCommands() {
    static ConCommand cmd_clear(
        "clear",
        [](const ConCommandArgs & /*args*/) { Console::Get().ClearLog(); },
        "Clear all console output");

    static ConCommand cmd_echo(
        "echo",
        [](const ConCommandArgs &args) {
          std::string msg;
          for (int i = 1; i < args.ArgC(); i++) {
            msg += args[i] + (i < args.ArgC() - 1 ? " " : "");
          }
          S67_CORE_INFO("{0}", msg);
        },
        "Echo text to console");

    static ConCommand cmd_list(
        "list",
        [](const ConCommandArgs &args) {
          S67_CORE_INFO("--- Command List ---");
          const auto &commands = Console::Get().GetCommands();
          for (const auto &[name, cmd] : commands) {
            S67_CORE_INFO("{0} : {1}", name, cmd->GetHelpString());
          }
          S67_CORE_INFO("--- Variable List ---");
          const auto &vars = Console::Get().GetConVars();
          for (const auto &[name, var] : vars) {
            S67_CORE_INFO("{0} = \"{1}\" : {2}", name, var->GetString(),
                          var->GetHelpString());
          }
        },
        "List all commands and variables");
  }
};

static StandardCommands s_StandardCommands;

} // namespace S67
