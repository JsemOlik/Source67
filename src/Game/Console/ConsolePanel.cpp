#include "ConsolePanel.h"
#include "Console.h"
#include "Core/Logger.h"
#include <imgui.h>
#include <imgui_internal.h>

namespace S67 {

ConsolePanel::ConsolePanel() {
  memset(m_InputBuffer, 0, sizeof(m_InputBuffer));
}

void ConsolePanel::OnImGuiRender(bool *pOpen) {
  if (!*pOpen)
    return;

  ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);

  // Source-like styling (no docking to keep it separate)
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;

  // Keep the semi-transparent dark background for that "Game Console" feel
  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.95f));
  ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed,
                        ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

  if (!ImGui::Begin("Developer Console", pOpen, window_flags)) {
    ImGui::PopStyleColor(4);
    ImGui::End();
    return;
  }

  // 1. Output Region
  const float footer_height_to_reserve =
      ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
  ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve),
                    false, ImGuiWindowFlags_HorizontalScrollbar);

  // Display logs from Core::Logger
  auto logs = Logger::GetLogHistory();

  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tight spacing

  for (const auto &log : logs) {
    ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    // Syntax Highlighting Logic
    if (log.Level == spdlog::level::trace) {
      if (log.Message.starts_with("] ")) {
        color = ImVec4(0.0f, 1.0f, 1.0f, 1.0f); // Cyan for User Input
      } else {
        color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f); // Grey for trace
      }
    } else if (log.Level == spdlog::level::info) {
      // Check if it looks like a variable output "var = value : help"
      if (log.Message.find(" = \"") != std::string::npos) {
        color = ImVec4(0.2f, 1.0f, 0.2f, 1.0f); // Green for ConVars
      } else {
        color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White for Info
      }
    } else if (log.Level == spdlog::level::warn) {
      color = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
    } else if (log.Level == spdlog::level::err) {
      color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
    } else if (log.Level == spdlog::level::critical) {
      color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    }

    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::TextUnformatted(log.Message.c_str());
    ImGui::PopStyleColor();
  }

  if (m_ScrollToBottom || (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
    ImGui::SetScrollHereY(1.0f);
  m_ScrollToBottom = false;

  ImGui::PopStyleVar();
  ImGui::EndChild();

  ImGui::Separator();

  // 2. Input Region
  bool reclaim_focus = false;
  ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_EnterReturnsTrue |
                                    ImGuiInputTextFlags_CallbackCompletion |
                                    ImGuiInputTextFlags_CallbackHistory;

  ImGui::PushItemWidth(-1);
  if (ImGui::InputText(
          "##Input", m_InputBuffer, sizeof(m_InputBuffer), input_flags,
          [](ImGuiInputTextCallbackData *data) {
            return ((ConsolePanel *)data->UserData)->TextEditCallback(data);
          },
          (void *)this)) {
    std::string command = m_InputBuffer;
    if (!command.empty()) {
      Console::Get().ExecuteCommand(command);

      // Add to history
      m_HistoryPos = -1;
      if (m_History.empty() || m_History.back() != command) {
        m_History.push_back(command);
      }

      memset(m_InputBuffer, 0, sizeof(m_InputBuffer));
      reclaim_focus = true;
      m_ScrollToBottom = true;
    }
  }
  ImGui::PopItemWidth();

  // Auto-focus on window appearing
  ImGui::SetItemDefaultFocus();
  if (reclaim_focus)
    ImGui::SetKeyboardFocusHere(-1); // Focus previous widget

  ImGui::PopStyleColor(4);
  ImGui::End();
}

int ConsolePanel::TextEditCallback(void *data) {
  ImGuiInputTextCallbackData *my_data = (ImGuiInputTextCallbackData *)data;

  switch (my_data->EventFlag) {
  case ImGuiInputTextFlags_CallbackCompletion: {
    // TODO: Auto-completion
    break;
  }
  case ImGuiInputTextFlags_CallbackHistory: {
    const int prev_history_pos = m_HistoryPos;
    if (my_data->EventKey == ImGuiKey_UpArrow) {
      if (m_HistoryPos == -1)
        m_HistoryPos = (int)m_History.size() - 1;
      else if (m_HistoryPos > 0)
        m_HistoryPos--;
    } else if (my_data->EventKey == ImGuiKey_DownArrow) {
      if (m_HistoryPos != -1)
        if (++m_HistoryPos >= m_History.size())
          m_HistoryPos = -1;
    }

    if (prev_history_pos != m_HistoryPos) {
      std::string history_str =
          (m_HistoryPos >= 0) ? m_History[m_HistoryPos] : "";
      my_data->DeleteChars(0, my_data->BufTextLen);
      my_data->InsertChars(0, history_str.c_str());
    }
    break;
  }
  }
  return 0;
}

} // namespace S67
