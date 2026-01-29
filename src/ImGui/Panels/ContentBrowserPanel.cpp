#include "ContentBrowserPanel.h"
#include "Core/Application.h"
#include "Core/PlatformUtils.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <imgui.h>
#include <string>

namespace S67 {

static const std::filesystem::path s_AssetPath = "assets";

ContentBrowserPanel::ContentBrowserPanel()
    : m_BaseDirectory(s_AssetPath), m_CurrentDirectory(s_AssetPath) {

  std::filesystem::path levelIconPath =
      Application::Get().ResolveAssetPath("assets/engine/level_icon.png");
  if (std::filesystem::exists(levelIconPath)) {
    m_LevelIcon = Texture2D::Create(levelIconPath.string());
  }

  std::filesystem::path folderIconPath =
      Application::Get().ResolveAssetPath("assets/engine/folder_icon.png");
  if (std::filesystem::exists(folderIconPath)) {
    m_FolderIcon = Texture2D::Create(folderIconPath.string());
  }

  std::filesystem::path backIconPath =
      Application::Get().ResolveAssetPath("assets/engine/back_arrow_icon.png");
  if (std::filesystem::exists(backIconPath)) {
    m_BackArrowIcon = Texture2D::Create(backIconPath.string());
  }
}

void ContentBrowserPanel::SetRoot(const std::filesystem::path &root) {
  m_BaseDirectory = root;
  m_CurrentDirectory = root;
  m_ThumbnailCache.clear();
}

void ContentBrowserPanel::OnImGuiRender() {
  ImGui::Begin("Content Browser");

  // --- Top Bar ---
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {4, 4});
  if (ImGui::Button("+ Add")) {
    ImGui::OpenPopup("AddMenu");
  }
  if (ImGui::BeginPopup("AddMenu")) {
    if (ImGui::MenuItem("New Level")) {
      std::filesystem::path newPath = m_CurrentDirectory / "NewLevel.s67";
      int i = 1;
      while (std::filesystem::exists(newPath)) {
        newPath =
            m_CurrentDirectory / ("NewLevel_" + std::to_string(i++) + ".s67");
      }
      CreateDefaultLevel(newPath);
    }
    ImGui::EndPopup();
  }
  ImGui::SameLine();
  if (ImGui::Button("Import")) {
    std::string path = FileDialogs::OpenFile("All Files (*.*)\0*.*\0", "*");
    if (!path.empty()) {
      std::filesystem::path src(path);
      std::filesystem::path dst = m_CurrentDirectory / src.filename();
      if (std::filesystem::exists(dst)) {
        std::string base = src.stem().string();
        std::string ext = src.extension().string();
        int i = 1;
        while (std::filesystem::exists(dst)) {
          dst = m_CurrentDirectory / (base + "_" + std::to_string(i++) + ext);
        }
      }
      try {
        std::filesystem::copy(src, dst);
      } catch (...) {
      }
    }
  }
  ImGui::SameLine();
  ImGui::PushItemWidth(200);
  if (ImGui::InputTextWithHint("##Search", "Search Content", m_SearchBuffer,
                               sizeof(m_SearchBuffer))) {
  }
  ImGui::PopItemWidth();
  ImGui::PopStyleVar();

  ImGui::Separator();

  // --- Sidebar & Grid Split ---
  static float sidebarWidth = 200.0f;
  ImGui::BeginChild("Sidebar", ImVec2(sidebarWidth, 0), true);
  if (ImGui::CollapsingHeader("Content", ImGuiTreeNodeFlags_DefaultOpen)) {
    RenderDirectoryTree(m_BaseDirectory);
  }
  ImGui::EndChild();

  ImGui::SameLine();

  ImGui::BeginChild("GridContent", ImVec2(0, 0), false);

  // --- Breadcrumbs ---
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {2, 0});
  std::vector<std::filesystem::path> components;
  std::filesystem::path curr = m_CurrentDirectory;
  while (curr != m_BaseDirectory.parent_path() && !curr.empty()) {
    components.push_back(curr);
    curr = curr.parent_path();
  }
  std::reverse(components.begin(), components.end());

  for (size_t i = 0; i < components.size(); i++) {
    if (i > 0) {
      ImGui::TextDisabled(">");
      ImGui::SameLine();
    }
    if (ImGui::Selectable(
            components[i].filename().string().c_str(), false, 0,
            ImGui::CalcTextSize(components[i].filename().string().c_str()))) {
      m_CurrentDirectory = components[i];
    }
    ImGui::SameLine();
  }
  ImGui::PopStyleVar();
  ImGui::NewLine();
  ImGui::Separator();

  // --- Grid ---
  static float padding = 16.0f;
  static float thumbnailSize = 120.0f;
  float cellSize = thumbnailSize + padding;

  float panelWidth = ImGui::GetContentRegionAvail().x;
  int columnCount = (int)(panelWidth / cellSize);
  if (columnCount < 1)
    columnCount = 1;

  ImGui::Columns(columnCount, 0, false);

  std::string searchFilter = m_SearchBuffer;
  std::transform(searchFilter.begin(), searchFilter.end(), searchFilter.begin(),
                 ::tolower);

  try {
    for (auto &entry :
         std::filesystem::directory_iterator(m_CurrentDirectory)) {
      const auto &path = entry.path();
      std::string filename = path.filename().string();
      if (filename.empty())
        continue;

      // Filter by search
      if (!searchFilter.empty()) {
        std::string lowerFilename = filename;
        std::transform(lowerFilename.begin(), lowerFilename.end(),
                       lowerFilename.begin(), ::tolower);
        if (lowerFilename.find(searchFilter) == std::string::npos)
          continue;
      }

      ImGui::PushID(filename.c_str());

      bool isDir = entry.is_directory();
      bool isLevel = path.extension() == ".s67";
      bool isImage = path.extension() == ".png" || path.extension() == ".jpg" ||
                     path.extension() == ".jpeg";

      ImTextureID iconID = 0;
      if (isDir && m_FolderIcon)
        iconID = (ImTextureID)(uint64_t)m_FolderIcon->GetRendererID();
      else if (isLevel && m_LevelIcon)
        iconID = (ImTextureID)(uint64_t)m_LevelIcon->GetRendererID();
      else if (isImage) {
        if (m_ThumbnailCache.find(path.string()) == m_ThumbnailCache.end()) {
          auto tex = Texture2D::Create(path.string());
          if (tex)
            m_ThumbnailCache[path.string()] = tex;
        }
        if (m_ThumbnailCache.count(path.string()) &&
            m_ThumbnailCache[path.string()]) {
          iconID = (ImTextureID)(uint64_t)m_ThumbnailCache[path.string()]
                       ->GetRendererID();
        }
      }

      if (iconID != 0) {
        ImVec2 pos = ImGui::GetCursorScreenPos();
        bool hovered = ImGui::IsMouseHoveringRect(
            pos, {pos.x + thumbnailSize, pos.y + thumbnailSize});
        ImVec4 tint = hovered ? ImVec4{1.2f, 1.2f, 1.2f, 1.0f}
                              : ImVec4{1.0f, 1.0f, 1.0f, 1.0f};

        if (isDir) {
          ImGui::PushStyleColor(ImGuiCol_Button, {0, 0, 0, 0});
          ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0, 0, 0, 0});
          ImGui::PushStyleColor(ImGuiCol_ButtonActive, {0, 0, 0, 0});
          ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

          ImGui::ImageButton("##folder", iconID, {thumbnailSize, thumbnailSize},
                             {0, 1}, {1, 0}, {0, 0, 0, 0}, tint);

          ImGui::PopStyleVar();
          ImGui::PopStyleColor(3);
        } else {
          ImGui::ImageButton("##asset", iconID, {thumbnailSize, thumbnailSize},
                             {0, 1}, {1, 0});
        }
      } else {
        std::string label = isDir ? "[D]" : (isLevel ? "[L]" : "[F]");
        ImGui::Button(label.c_str(), {thumbnailSize, thumbnailSize});
      }

      if (ImGui::BeginDragDropSource()) {
        std::string p = path.string();
        ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", p.c_str(),
                                  (p.length() + 1) * sizeof(char));
        ImGui::Text("%s", filename.c_str());
        ImGui::EndDragDropSource();
      }

      if (isDir && ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload *payload =
                ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
          std::filesystem::path sourcePath((const char *)payload->Data);
          if (sourcePath.parent_path() != path) {
            std::filesystem::path destPath = path / sourcePath.filename();
            try {
              std::filesystem::rename(sourcePath, destPath);
            } catch (...) {
            }
          }
        }
        ImGui::EndDragDropTarget();
      }

      if (ImGui::IsItemHovered() &&
          ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        if (isDir)
          m_CurrentDirectory /= path.filename();
        else if (isLevel)
          Application::Get().OpenScene(path.string());
        else
          FileDialogs::OpenExternally(path.string());
      }

      if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Open in Finder"))
          FileDialogs::OpenExplorer(path.string());

        if (ImGui::MenuItem("Rename")) {
          m_PathToRename = path;
          std::strncpy(m_RenameBuffer, path.filename().string().c_str(),
                       sizeof(m_RenameBuffer));
          m_ShowRenameModal = true;
        }

        if (ImGui::MenuItem("Delete")) {
          m_PathToDelete = path;
          m_ShowDeleteModal = true;
        }
        ImGui::EndPopup();
      }

      ImGui::TextWrapped("%s", filename.c_str());
      ImGui::NextColumn();
      ImGui::PopID();
    }
  } catch (...) {
  }

  ImGui::Columns(1);

  if (ImGui::BeginPopupContextWindow(nullptr,
                                     ImGuiPopupFlags_MouseButtonRight |
                                         ImGuiPopupFlags_NoOpenOverItems)) {
    if (ImGui::MenuItem("Create New Folder")) {
      std::filesystem::path newPath = m_CurrentDirectory / "NewFolder";
      int i = 1;
      while (std::filesystem::exists(newPath)) {
        newPath = m_CurrentDirectory / ("NewFolder_" + std::to_string(i++));
      }
      std::filesystem::create_directory(newPath);
    }
    if (ImGui::MenuItem("Create New Level")) {
      std::filesystem::path newPath = m_CurrentDirectory / "NewLevel.s67";
      int i = 1;
      while (std::filesystem::exists(newPath)) {
        newPath =
            m_CurrentDirectory / ("NewLevel_" + std::to_string(i++) + ".s67");
      }
      CreateDefaultLevel(newPath);
    }
    ImGui::EndPopup();
  }

  if (m_ShowDeleteModal) {
    ImGui::OpenPopup("Delete Asset?");
    m_ShowDeleteModal = false;
  }
  if (ImGui::BeginPopupModal("Delete Asset?", NULL,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Are you sure you want to delete '%s'?",
                m_PathToDelete.filename().string().c_str());
    if (ImGui::Button("Delete", {120, 0})) {
      std::filesystem::remove_all(m_PathToDelete);
      m_PathToDelete = "";
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", {120, 0})) {
      m_PathToDelete = "";
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  if (m_ShowRenameModal) {
    ImGui::OpenPopup("Rename Folder");
    m_ShowRenameModal = false;
  }
  if (ImGui::BeginPopupModal("Rename Folder", NULL,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::InputText("New Name", m_RenameBuffer, sizeof(m_RenameBuffer));
    if (ImGui::Button("Rename", {120, 0})) {
      std::filesystem::path newPath =
          m_PathToRename.parent_path() / m_RenameBuffer;
      if (!std::filesystem::exists(newPath)) {
        std::filesystem::rename(m_PathToRename, newPath);
        m_PathToRename = "";
        ImGui::CloseCurrentPopup();
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", {120, 0})) {
      m_PathToRename = "";
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  ImGui::EndChild(); // Close GridContent
  ImGui::End();
}

void ContentBrowserPanel::RenderDirectoryTree(
    const std::filesystem::path &directoryPath) {
  std::string filename = directoryPath.filename().string();
  if (filename.empty())
    filename = directoryPath.string();

  ImGuiTreeNodeFlags flags =
      ((m_CurrentDirectory == directoryPath) ? ImGuiTreeNodeFlags_Selected
                                             : 0) |
      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

  // Check if directory has subdirectories
  bool hasSubdirs = false;
  try {
    for (auto &entry : std::filesystem::directory_iterator(directoryPath)) {
      if (entry.is_directory()) {
        hasSubdirs = true;
        break;
      }
    }
  } catch (...) {
  }

  if (!hasSubdirs)
    flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

  bool opened = ImGui::TreeNodeEx((void *)hash_value(directoryPath), flags,
                                  filename.c_str());

  if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
    m_CurrentDirectory = directoryPath;
  }

  if (opened && hasSubdirs) {
    try {
      for (auto &entry : std::filesystem::directory_iterator(directoryPath)) {
        if (entry.is_directory()) {
          RenderDirectoryTree(entry.path());
        }
      }
    } catch (...) {
    }
    ImGui::TreePop();
  }
}

void ContentBrowserPanel::CreateDefaultLevel(
    const std::filesystem::path &path) {
  std::ofstream f(path);
  if (!f.is_open())
    return;

  f << "Scene: " << path.stem().string() << "\n";
  f << "Entities:\n";

  // 1. Floor (Anchored)
  f << "  - Entity: Floor\n";
  f << "    Transform:\n";
  f << "      Position: [0, -2, 0]\n";
  f << "      Rotation: [0, 0, 0]\n";
  f << "      Scale: [20, 1, 20]\n";
  f << "    MeshPath: Cube\n";
  f << "    ShaderPath: assets/shaders/Lighting.glsl\n";
  f << "    TexturePath: assets/textures/Checkerboard.png\n";
  f << "    TextureTiling: [1, 1]\n";
  f << "    Collidable: true\n";
  f << "    Anchored: true\n";

  // 2. Dynamic Cubes
  for (int i = 0; i < 5; i++) {
    f << "  - Entity: Cube " << i << "\n";
    f << "    Transform:\n";
    f << "      Position: [" << (float)i * 2.0f - 4.0f << ", "
      << 10.0f + (float)i * 2.0f << ", 0]\n";
    f << "      Rotation: [0, 0, 0]\n";
    f << "      Scale: [1, 1, 1]\n";
    f << "    MeshPath: Cube\n";
    f << "    ShaderPath: assets/shaders/Lighting.glsl\n";
    f << "    TexturePath: assets/textures/Checkerboard.png\n";
    f << "    TextureTiling: [1, 1]\n";
    f << "    Collidable: true\n";
    f << "    Anchored: false\n";
  }

  // 3. Player
  f << "  - Entity: Player\n";
  f << "    Transform:\n";
  f << "      Position: [0, 2, 0]\n";
  f << "      Rotation: [0, 0, 0]\n";
  f << "      Scale: [1, 1.5, 1]\n";
  f << "    MeshPath: Cube\n";
  f << "    ShaderPath: assets/shaders/Lighting.glsl\n";
  f << "    TexturePath: assets/textures/Debug.png\n";
  f << "    TextureTiling: [1, 1]\n";
  f << "    Collidable: true\n";
  f << "    CameraFOV: 45\n";

  f.close();
}

} // namespace S67
