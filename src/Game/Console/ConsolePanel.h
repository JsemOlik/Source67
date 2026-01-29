#pragma once
#include <string>
#include <vector>


namespace S67 {

class ConsolePanel {
public:
  ConsolePanel();
  ~ConsolePanel() = default;

  void OnImGuiRender(bool *pOpen);

private:
  char m_InputBuffer[256];
  bool m_ScrollToBottom = true;

  // Command History (Up/Down arrow)
  std::vector<std::string> m_History; // Commands typed
  int m_HistoryPos = -1; // -1: new line, 0..Size-1: browsing history

  int TextEditCallback(void *data);
};

} // namespace S67
