#include "ContentBrowserPanel.h"
#include <imgui.h>
#include "Core/Application.h"

namespace S67 {

    // For now, project root is current directory
    static const std::filesystem::path s_AssetPath = "assets";

    ContentBrowserPanel::ContentBrowserPanel()
        : m_BaseDirectory(s_AssetPath), m_CurrentDirectory(s_AssetPath) {
    }

    void ContentBrowserPanel::SetRoot(const std::filesystem::path& root) {
        m_BaseDirectory = root;
        m_CurrentDirectory = root;
        m_ThumbnailCache.clear();
    }

    void ContentBrowserPanel::OnImGuiRender() {
        ImGui::Begin("Content Browser");

        if (m_CurrentDirectory != m_BaseDirectory) {
            if (ImGui::Button("<-")) {
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
            }
        }

        static float padding = 16.0f;
        static float thumbnailSize = 120.0f;
        float cellSize = thumbnailSize + padding;

        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / cellSize);
        if (columnCount < 1) columnCount = 1;

        ImGui::Columns(columnCount, 0, false);

        for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory)) {
            const auto& path = directoryEntry.path();
            std::string filenameString = path.filename().string();

            ImGui::PushID(filenameString.c_str());
            
            bool isDirectory = directoryEntry.is_directory();
            bool isImage = path.extension() == ".png" || path.extension() == ".jpg";
            
            ImTextureID iconID = 0; // Use dummy or fallback
            if (isImage) {
                if (m_ThumbnailCache.find(path.string()) == m_ThumbnailCache.end()) {
                    m_ThumbnailCache[path.string()] = Texture2D::Create(path.string());
                }
                iconID = (ImTextureID)(uint64_t)m_ThumbnailCache[path.string()]->GetRendererID();
            }

            if (iconID) {
                ImGui::ImageButton(filenameString.c_str(), iconID, { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
            } else {
                ImGui::Button(isDirectory ? "[D]" : "[F]", { thumbnailSize, thumbnailSize });
            }

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                if (isDirectory) {
                    m_CurrentDirectory /= path.filename();
                } else if (path.extension() == ".s67") {
                    Application::Get().OpenScene(path.string());
                }
            }
            ImGui::TextWrapped("%s", filenameString.c_str());

            ImGui::NextColumn();
            ImGui::PopID();
        }

        ImGui::Columns(1);
        ImGui::End();
    }

}
