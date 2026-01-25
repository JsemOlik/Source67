#include "ContentBrowserPanel.h"
#include <imgui.h>
#include "Core/Application.h"
#include "Core/PlatformUtils.h"

namespace S67 {

    // For now, project root is current directory
    static const std::filesystem::path s_AssetPath = "assets";

    ContentBrowserPanel::ContentBrowserPanel()
        : m_BaseDirectory(s_AssetPath), m_CurrentDirectory(s_AssetPath) {
        
        std::filesystem::path iconPath = "assets/textures/level_icon.png";
        if (std::filesystem::exists(iconPath)) {
            m_LevelIcon = Texture2D::Create(iconPath.string());
        }
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
            std::string ext = path.extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            bool isImage = ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga";
            bool isLevel = ext == ".s67";
            bool isMesh = ext == ".obj" || ext == ".stl";
            
            ImTextureID iconID = 0; // Use dummy or fallback
            if (isImage) {
                if (m_ThumbnailCache.find(path.string()) == m_ThumbnailCache.end()) {
                    m_ThumbnailCache[path.string()] = Texture2D::Create(path.string());
                }
                iconID = (ImTextureID)(uint64_t)m_ThumbnailCache[path.string()]->GetRendererID();
            } else if (isLevel && m_LevelIcon) {
                iconID = (ImTextureID)(uint64_t)m_LevelIcon->GetRendererID();
            }

            if (iconID) {
                ImGui::ImageButton(filenameString.c_str(), iconID, { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
            } else {
                std::string label = isDirectory ? "[D]" : (isLevel ? "[L]" : (isMesh ? "[M]" : "[F]"));
                ImGui::Button(label.c_str(), { thumbnailSize, thumbnailSize });
            }

            if (ImGui::BeginDragDropSource()) {
                std::string pathString = path.string();
                const char* itemPath = pathString.c_str();
                ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (strlen(itemPath) + 1) * sizeof(char));
                ImGui::Text("%s", filenameString.c_str());
                ImGui::EndDragDropSource();
            }

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                if (isDirectory) {
                    m_CurrentDirectory /= path.filename();
                } else if (isLevel) {
                    Application::Get().OpenScene(path.string());
                } else {
                    FileDialogs::OpenExternally(path.string());
                }
            }

            // Right-click on item
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Open in Finder")) {
                    FileDialogs::OpenExplorer(path.string());
                }

                if (isDirectory) {
                    if (ImGui::MenuItem("Rename")) {
                        m_PathToRename = path;
                        std::strncpy(m_RenameBuffer, path.filename().string().c_str(), sizeof(m_RenameBuffer));
                        m_ShowRenameModal = true;
                    }
                }
                
                if (ImGui::MenuItem("Delete")) {
                    m_PathToDelete = path;
                    m_ShowDeleteModal = true;
                }
                ImGui::EndPopup();
            }

            ImGui::TextWrapped("%s", filenameString.c_str());

            ImGui::NextColumn();
            ImGui::PopID();
        }

        ImGui::Columns(1);

        // Right-click on background
        if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
            if (ImGui::MenuItem("Create New Folder")) {
                std::filesystem::path newPath = m_CurrentDirectory / "NewFolder";
                int i = 1;
                while (std::filesystem::exists(newPath)) {
                    newPath = m_CurrentDirectory / ("NewFolder_" + std::to_string(i++));
                }
                std::filesystem::create_directory(newPath);
            }

            if (ImGui::MenuItem("Open in Finder")) {
                FileDialogs::OpenExplorer(m_CurrentDirectory.string());
            }

            ImGui::EndPopup();
        }

        // Deletion Modal
        if (m_ShowDeleteModal) {
            ImGui::OpenPopup("Delete Asset?");
            m_ShowDeleteModal = false;
        }

        if (ImGui::BeginPopupModal("Delete Asset?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Are you sure you want to delete '%s'?", m_PathToDelete.filename().string().c_str());
            ImGui::TextColored({ 1.0f, 0.4f, 0.4f, 1.0f }, "This action cannot be undone!");
            ImGui::Separator();

            if (ImGui::Button("Delete", { 120, 0 })) {
                std::filesystem::remove_all(m_PathToDelete);
                m_PathToDelete = "";
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", { 120, 0 })) {
                m_PathToDelete = "";
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Rename Modal
        if (m_ShowRenameModal) {
            ImGui::OpenPopup("Rename Folder");
            m_ShowRenameModal = false;
        }

        if (ImGui::BeginPopupModal("Rename Folder", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::InputText("New Name", m_RenameBuffer, sizeof(m_RenameBuffer));
            ImGui::Separator();

            if (ImGui::Button("Rename", { 120, 0 })) {
                std::filesystem::path newPath = m_PathToRename.parent_path() / m_RenameBuffer;
                if (!std::filesystem::exists(newPath)) {
                    std::filesystem::rename(m_PathToRename, newPath);
                    m_PathToRename = "";
                    ImGui::CloseCurrentPopup();
                } else {
                   // Optional: Simple warning if name exists? I'll let it fail or just let user try again.
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", { 120, 0 })) {
                m_PathToRename = "";
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::End();
    }

}
