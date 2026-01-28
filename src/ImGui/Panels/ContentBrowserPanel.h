#pragma once

#include "Core/Base.h"
#include "Renderer/Texture.h"
#include <filesystem>
#include <unordered_map>

namespace S67 {

class ContentBrowserPanel {
public:
  ContentBrowserPanel();

  void OnImGuiRender();
  void SetRoot(const std::filesystem::path &root);

  const std::filesystem::path &GetCurrentDirectory() const {
    return m_CurrentDirectory;
  }

private:
  void RenderDirectoryTree(const std::filesystem::path &directoryPath);
  void CreateDefaultLevel(const std::filesystem::path &path);

  std::filesystem::path m_BaseDirectory;
  std::filesystem::path m_CurrentDirectory;
  std::unordered_map<std::string, Ref<Texture2D>> m_ThumbnailCache;

  char m_SearchBuffer[256] = {0};
  bool m_ShowSidebar = true;

  std::filesystem::path m_PathToDelete;
  bool m_ShowDeleteModal = false;

  std::filesystem::path m_PathToRename;
  char m_RenameBuffer[256];
  bool m_ShowRenameModal = false;

  Ref<Texture2D> m_LevelIcon;
  Ref<Texture2D> m_FolderIcon;
  Ref<Texture2D> m_BackArrowIcon;
};

} // namespace S67
