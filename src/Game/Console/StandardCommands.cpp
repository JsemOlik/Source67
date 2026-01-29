#include "ConVar.h"
#include "Console.h"
#include "Core/Application.h"
#include "Core/Logger.h"
#include <iomanip>
#include <sstream>

namespace S67 {

// Helper to register standard commands statically
struct StandardCommands {
  StandardCommands() {
    static ConCommand cmd_clear(
        "clear",
        [](const ConCommandArgs & /*args*/) { Logger::ClearLogHistory(); },
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

    static ConCommand cmd_writeconfig(
        "host_writeconfig",
        [](const ConCommandArgs & /*args*/) {
          Console::Get().Save("game.cfg");
        },
        "Save current configuration to game.cfg");

    static ConCommand cmd_map(
        "map",
        [](const ConCommandArgs &args) {
          if (args.ArgC() < 2) {
            S67_CORE_WARN("Usage: map <filename>");
            return;
          }
          std::string filename = args[1];
          // Auto-append extension if missing
          if (filename.find(".s67") == std::string::npos) {
            filename += ".s67";
          }
          S67_CORE_INFO("Loading map: {0}...", filename);

          // Resolve path to handle "mapname" vs "assets/levels/mapname"
          std::filesystem::path resolvedPath =
              Application::Get().ResolveAssetPath(filename);

          // If not found, try searching recursively or in common folders?
          // For now, let's just assume ResolveAssetPath checks Project/Assets
          // root. But ResolveAssetPath only checks direct concatenation. Let's
          // add a check for the file existence to give better error msg if
          // needed.

          Application::Get().OpenScene(resolvedPath.string());
        },
        "Load a map/scene by filename");
  }
};

static StandardCommands s_StandardCommands;

} // namespace S67
