#pragma once

#include <filesystem>
#include <unordered_map>
#include "Core/Base.h"
#include "Renderer/Texture.h"

namespace S67 {

    class ContentBrowserPanel {
    public:
        ContentBrowserPanel();

        void OnImGuiRender();
        void SetRoot(const std::filesystem::path& root);

    private:
        std::filesystem::path m_BaseDirectory;
        std::filesystem::path m_CurrentDirectory;
        std::unordered_map<std::string, Ref<Texture2D>> m_ThumbnailCache;

        std::filesystem::path m_PathToDelete;
        bool m_ShowDeleteModal = false;

        std::filesystem::path m_PathToRename;
        char m_RenameBuffer[256];
        bool m_ShowRenameModal = false;
    };

}
