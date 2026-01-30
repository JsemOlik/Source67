#include "ImGuiLayer.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <filesystem>
#include <imgui.h>

#include "Core/Application.h"
#include "Core/Logger.h"

#include <GLFW/glfw3.h>

namespace S67 {

void ImGuiLayer::SetDarkThemeColors() {
  auto &colors = ImGui::GetStyle().Colors;
  colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.105f, 0.11f, 1.0f};

  // Headers
  colors[ImGuiCol_Header] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
  colors[ImGuiCol_HeaderHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
  colors[ImGuiCol_HeaderActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

  // Buttons
  colors[ImGuiCol_Button] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
  colors[ImGuiCol_ButtonHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
  colors[ImGuiCol_ButtonActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

  // Frame BG
  colors[ImGuiCol_FrameBg] = ImVec4{0.13f, 0.135f, 0.14f, 1.0f};
  colors[ImGuiCol_FrameBgHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
  colors[ImGuiCol_FrameBgActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

  // Tabs
  colors[ImGuiCol_Tab] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
  colors[ImGuiCol_TabHovered] = ImVec4{0.38f, 0.3805f, 0.381f, 1.0f};
  colors[ImGuiCol_TabActive] = ImVec4{0.28f, 0.2805f, 0.281f, 1.0f};
  colors[ImGuiCol_TabUnfocused] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};

  // Title
  colors[ImGuiCol_TitleBg] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
  colors[ImGuiCol_TitleBgActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
}

void ImGuiLayer::SetDraculaThemeColors() {
  auto &colors = ImGui::GetStyle().Colors;

  colors[ImGuiCol_WindowBg] = ImVec4{0.157f, 0.165f, 0.212f, 1.0f};

  // Headers
  colors[ImGuiCol_Header] = ImVec4{0.267f, 0.278f, 0.353f, 1.0f};
  colors[ImGuiCol_HeaderHovered] = ImVec4{0.384f, 0.447f, 0.643f, 1.0f};
  colors[ImGuiCol_HeaderActive] = ImVec4{0.741f, 0.576f, 0.976f, 1.0f};

  // Buttons
  colors[ImGuiCol_Button] = ImVec4{0.267f, 0.278f, 0.353f, 1.0f};
  colors[ImGuiCol_ButtonHovered] = ImVec4{0.384f, 0.447f, 0.643f, 1.0f};
  colors[ImGuiCol_ButtonActive] = ImVec4{0.741f, 0.576f, 0.976f, 1.0f};

  // Frame BG
  colors[ImGuiCol_FrameBg] = ImVec4{0.176f, 0.184f, 0.235f, 1.0f};
  colors[ImGuiCol_FrameBgHovered] = ImVec4{0.267f, 0.278f, 0.353f, 1.0f};
  colors[ImGuiCol_FrameBgActive] = ImVec4{0.384f, 0.447f, 0.643f, 1.0f};

  // Tabs
  colors[ImGuiCol_Tab] = ImVec4{0.176f, 0.184f, 0.235f, 1.0f};
  colors[ImGuiCol_TabHovered] = ImVec4{0.384f, 0.447f, 0.643f, 1.0f};
  colors[ImGuiCol_TabActive] = ImVec4{0.267f, 0.278f, 0.353f, 1.0f};
  colors[ImGuiCol_TabUnfocused] = ImVec4{0.157f, 0.165f, 0.212f, 1.0f};
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.267f, 0.278f, 0.353f, 1.0f};

  // Title
  colors[ImGuiCol_TitleBg] = ImVec4{0.122f, 0.129f, 0.165f, 1.0f};
  colors[ImGuiCol_TitleBgActive] = ImVec4{0.157f, 0.165f, 0.212f, 1.0f};
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.122f, 0.129f, 0.165f, 1.0f};

  // Text
  colors[ImGuiCol_Text] = ImVec4{0.973f, 0.973f, 0.949f, 1.0f};
}

void ImGuiLayer::OnAttach() {
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  // ViewportsEnable removed to keep all windows inside main window
  io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;
  io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;

  // Load Font
  std::string fontPath = "assets/fonts/Roboto-Medium.ttf";
  if (std::filesystem::exists(fontPath)) {
    io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 18.0f);
  } else {
    // Try loading from PAK
    std::vector<uint8_t> fontData;
    if (Application::Get().GetPakAsset(fontPath, fontData)) {
      // ImGui takes ownership of the data if we don't handle it carefully,
      // but here we allocate a persistent buffer for it.
      void *data = malloc(fontData.size());
      memcpy(data, fontData.data(), fontData.size());
      io.Fonts->AddFontFromMemoryTTF(data, (int)fontData.size(), 18.0f);
      S67_CORE_INFO("Loaded font {0} from PAK", fontPath);
    }
  }

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  SetDarkThemeColors();

  ImGuiStyle &style = ImGui::GetStyle();
  style.WindowRounding = 5.0f;
  style.FrameRounding = 4.0f;
  style.PopupRounding = 4.0f;
  style.GrabRounding = 4.0f;
  style.TabRounding = 4.0f;
  style.FrameBorderSize = 1.0f;

  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  Application &app = Application::Get();
  GLFWwindow *window =
      static_cast<GLFWwindow *>(app.GetWindow().GetNativeWindow());

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 410");
}

void ImGuiLayer::OnDetach() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void ImGuiLayer::OnEvent(Event &e) {
  if (m_BlockEvents && ImGui::GetCurrentContext()) {
    ImGuiIO &io = ImGui::GetIO();
    e.Handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
    e.Handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
  }
}

void ImGuiLayer::Begin() {
  if (!ImGui::GetCurrentContext()) {
    S67_CORE_ERROR("ImGui context not initialized!");
    return;
  }

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // DockSpace Setup
  static bool dockspaceOpen = true;
  static bool opt_fullscreen_persistant = true;
  bool opt_fullscreen = opt_fullscreen_persistant;
  static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
  if (opt_fullscreen) {
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |=
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
  }

  if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
    window_flags |= ImGuiWindowFlags_NoBackground;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
  ImGui::PopStyleVar();

  if (opt_fullscreen)
    ImGui::PopStyleVar(2);

  // DockSpace
  ImGuiIO &io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
  }
}

void ImGuiLayer::End() {
  if (!ImGui::GetCurrentContext())
    return;

  ImGuiIO &io = ImGui::GetIO();
  Application &app = Application::Get();
  // io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(),
  // (float)app.GetWindow().GetHeight()); // Let backend handle this

  ImGui::End(); // End of DockSpace window

  // Rendering
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    GLFWwindow *backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);
  }
}

} // namespace S67
