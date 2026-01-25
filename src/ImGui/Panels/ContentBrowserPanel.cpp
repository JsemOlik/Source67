#include "ContentBrowserPanel.h"
#include <imgui.h>
#include "Core/Application.h"

namespace S67 {

    // For now, project root is current directory
    static const std::filesystem::path s_AssetPath = "assets";

    ContentBrowserPanel::ContentBrowserPanel()
        : m_CurrentDirectory(s_AssetPath) {
    }

    void ContentBrowserPanel::OnImGuiRender() {
        ImGui::Begin("Content Browser");

        if (m_CurrentDirectory != std::filesystem::path(s_AssetPath)) {
            if (ImGui::Button("<-")) {
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
            }
        }

        static float padding = 16.0f;
        static float thumbnailSize = 128.0f;
        float cellSize = thumbnailSize + padding;

        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / cellSize);
        if (columnCount < 1) columnCount = 1;

        ImGui::Columns(columnCount, 0, false);

        for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory)) {
            const auto& path = directoryEntry.path();
            auto relativePath = std::filesystem::relative(path, s_AssetPath);
            std::string filenameString = path.filename().string();

            ImGui::PushID(filenameString.c_str());
            
            // Simple visual distinction
            bool isDirectory = directoryEntry.is_directory();
            ImGui::Button(isDirectory ? "[D]" : "[F]", { thumbnailSize, thumbnailSize });

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                if (isDirectory) {
                    m_CurrentDirectory /= path.filename();
                } else if (path.extension() == ".l67") {
                    Application::Get().OpenScene(path.string());
                }
            }
            ImGui::TextWrapped(filenameString.c_str());

            ImGui::NextColumn();
            ImGui::PopID();
        }

        ImGui::Columns(1);

        // ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
        // ImGui::SliderFloat("Padding", &padding, 0, 32);

        ImGui::End();
    }

}
