#include "ContentBrowserPanel.h"
#include "Core/Application.h"
#include "Core/PlatformUtils.h"
#include <imgui.h>

namespace S67 {

static const std::filesystem::path s_AssetPath = "assets";

ContentBrowserPanel::ContentBrowserPanel()
    : m_BaseDirectory(s_AssetPath), m_CurrentDirectory(s_AssetPath) {

  std::filesystem::path levelIconPath = "assets/engine/level_icon.png";
  if (std::filesystem::exists(levelIconPath)) {
    m_LevelIcon = Texture2D::Create(levelIconPath.string());
  }

  std::filesystem::path folderIconPath = "assets/engine/folder_icon.png";
  if (std::filesystem::exists(folderIconPath)) {
    m_FolderIcon = Texture2D::Create(folderIconPath.string());
  }

  std::filesystem::path backIconPath = "assets/engine/back_arrow_icon.png";
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

  static float padding = 16.0f;
  static float thumbnailSize = 120.0f;
  float cellSize = thumbnailSize + padding;

  float panelWidth = ImGui::GetContentRegionAvail().x;
  int columnCount = (int)(panelWidth / cellSize);
  if (columnCount < 1)
    columnCount = 1;

  ImGui::Columns(columnCount, 0, false);

  if (m_CurrentDirectory != m_BaseDirectory) {
    ImGui::PushID("back_nav_item");

    ImTextureID iconID =
        m_BackArrowIcon
            ? (ImTextureID)(uint64_t)m_BackArrowIcon->GetRendererID()
            : (m_FolderIcon
                   ? (ImTextureID)(uint64_t)m_FolderIcon->GetRendererID()
                   : 0);
    if (iconID != 0) {
      // Keep the button surface at thumbnailSize by using custom padding
      float iconScale =
          0.75f; // Final fine-tune: Scale to 0.75 (a tiny bit bigger)
      float iconSize = thumbnailSize * iconScale;
      float internalPadding = (thumbnailSize - iconSize) * 0.5f;

      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
                          {internalPadding, internalPadding});
      ImGui::ImageButton("##back_nav", iconID, {iconSize, iconSize}, {0, 1},
                         {1, 0});
      ImGui::PopStyleVar();
    } else {
      ImGui::Button("<-", {thumbnailSize, thumbnailSize});
    }

    if (ImGui::IsItemHovered() &&
        ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
      m_CurrentDirectory = m_CurrentDirectory.parent_path();
    }

    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload *payload =
              ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
        std::filesystem::path sourcePath((const char *)payload->Data);
        std::filesystem::path parentDir = m_CurrentDirectory.parent_path();
        std::filesystem::path destPath = parentDir / sourcePath.filename();

        if (sourcePath.parent_path() != parentDir) {
          if (std::filesystem::exists(destPath)) {
            std::string baseName = sourcePath.stem().string();
            std::string extension = sourcePath.extension().string();
            int counter = 1;
            do {
              destPath = parentDir / (baseName + "_" +
                                      std::to_string(counter++) + extension);
            } while (std::filesystem::exists(destPath));
          }
          try {
            std::filesystem::rename(sourcePath, destPath);
          } catch (...) {
          }
        }
      }
      ImGui::EndDragDropTarget();
    }

    ImGui::TextWrapped("");
    ImGui::NextColumn();
    ImGui::PopID();
  }

  try {
    for (auto &entry :
         std::filesystem::directory_iterator(m_CurrentDirectory)) {
      const auto &path = entry.path();
      std::string filename = path.filename().string();
      if (filename.empty())
        continue;

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
        if (isDir) {
          if (ImGui::MenuItem("Rename")) {
            m_PathToRename = path;
            std::strncpy(m_RenameBuffer, path.filename().string().c_str(),
                         sizeof(m_RenameBuffer));
            m_ShowRenameModal = true;
          }
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

  ImGui::End();
}

} // namespace S67
