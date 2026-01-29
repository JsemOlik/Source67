#pragma once
#include <functional>
#include <string>
#include <vector>


namespace S67 {

struct ConCommandArgs {
  std::vector<std::string> Args;
  int ArgC() const { return (int)Args.size(); }
  const std::string &Arg(int i) const {
    static std::string empty = "";
    if (i >= 0 && i < Args.size())
      return Args[i];
    return empty;
  }
  const std::string &operator[](int i) const { return Arg(i); }
};

using ConCommandCallback = std::function<void(const ConCommandArgs &args)>;

class ConCommand {
public:
  ConCommand(const std::string &name, ConCommandCallback callback,
             const std::string &helpString = "", int flags = 0);
  ~ConCommand();

  std::string GetName() const { return m_Name; }
  std::string GetHelpString() const { return m_HelpString; }
  int GetFlags() const { return m_Flags; }

  void Execute(const ConCommandArgs &args);

private:
  std::string m_Name;
  std::string m_HelpString;
  int m_Flags;
  ConCommandCallback m_Callback;
};

} // namespace S67
