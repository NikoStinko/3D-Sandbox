#include "MapPanel.h"
#include "EditorState.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <filesystem>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

namespace {
std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return s;
}

bool matchesFilter(const std::string& value, const std::string& filter) {
    if (filter.empty()) return true;
    return toLower(value).find(toLower(filter)) != std::string::npos;
}
}

MapPanel::MapPanel(const std::string& root,
                   SaveCallback saveCb,
                   LoadCallback loadCb,
                   NewCallback newCb,
                   EditorState* editorState)
    : mapsRoot(root)
    , save(std::move(saveCb))
    , load(std::move(loadCb))
    , createNew(std::move(newCb))
    , editor(editorState)
{
    scanFiles();
}

void MapPanel::rescan()
{
    scanFiles();
}

void MapPanel::draw(bool* open)
{
    if (!open || !(*open)) return;
    if (ImGui::Begin("Maps", open)) {
        if (ImGui::Button("Rescan")) {
            rescan();
        }
        ImGui::SameLine();
        ImGui::InputText("Filter", &filter);

        ImGui::TextDisabled("Root: %s", mapsRoot.c_str());
        if (editor) {
            ImGui::Text("Current: %s", editor->currentMapName.c_str());
        }

        ImGui::Separator();
        ImGui::BeginChild("maps_list", ImVec2(0, 180), true);
        for (int i = 0; i < static_cast<int>(files.size()); ++i) {
            const std::string& path = files[i];
            std::string name = displayName(path);
            if (!matchesFilter(name, filter)) continue;
            bool selectedEntry = (i == selected);
            if (ImGui::Selectable(name.c_str(), selectedEntry)) {
                selected = i;
            }
        }
        ImGui::EndChild();

        bool hasSelection = selected >= 0 && selected < static_cast<int>(files.size());
        std::string activePath = hasSelection ? files[selected] : pathForCurrentMap();

        if (ImGui::Button("Save")) {
            if (save && !activePath.empty()) {
                if (save(activePath)) {
                    scanFiles();
                    selectPath(activePath);
                }
            }
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(!hasSelection || !load);
        if (ImGui::Button("Load") && hasSelection) {
            if (load(files[selected])) {
                selectPath(files[selected]);
            }
        }
        ImGui::EndDisabled();

        ImGui::Separator();
        ImGui::InputText("New map name", &newMapName);
        ImGui::SameLine();
        if (ImGui::Button("Create New")) {
            if (createNew) {
                std::string newPath = resolvePath(newMapName.empty() ? "new_map" : newMapName);
                if (!newPath.empty() && createNew(newPath)) {
                    scanFiles();
                    selectPath(newPath);
                }
            }
        }
    }
    ImGui::End();
}

void MapPanel::scanFiles()
{
    files.clear();
    std::error_code ec;
    if (!mapsRoot.empty() && fs::exists(mapsRoot, ec)) {
        for (auto& entry : fs::directory_iterator(mapsRoot, ec)) {
            if (entry.is_regular_file()) {
                std::string ext = toLower(entry.path().extension().string());
                if (ext == ".json") {
                    files.push_back(entry.path().string());
                }
            }
        }
    }
    std::sort(files.begin(), files.end());
    if (selected >= static_cast<int>(files.size())) {
        selected = files.empty() ? -1 : static_cast<int>(files.size()) - 1;
    }
}

std::string MapPanel::selectedPath() const
{
    if (selected >= 0 && selected < static_cast<int>(files.size())) {
        return files[selected];
    }
    return std::string();
}

std::string MapPanel::resolvePath(const std::string& name) const
{
    if (name.empty()) return std::string();
    std::string sanitized = name;
    sanitized.erase(std::remove_if(sanitized.begin(), sanitized.end(), [](unsigned char c){ return std::isspace(c); }), sanitized.end());
    if (sanitized.empty()) return std::string();
    fs::path p = mapsRoot;
    fs::path filename = sanitized;
    if (filename.extension() != ".json") {
        filename.replace_extension(".json");
    }
    p /= filename;
    return p.string();
}

std::string MapPanel::displayName(const std::string& path) const
{
    return fs::path(path).filename().string();
}

std::string MapPanel::pathForCurrentMap() const
{
    if (!editor || editor->currentMapName.empty()) return std::string();
    return resolvePath(editor->currentMapName);
}

void MapPanel::selectPath(const std::string& path)
{
    for (int i = 0; i < static_cast<int>(files.size()); ++i) {
        if (files[i] == path) {
            selected = i;
            break;
        }
    }
}
