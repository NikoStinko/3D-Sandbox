#include "ModelBrowserPanel.h"
#include "ModelManager.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

ModelBrowserPanel::ModelBrowserPanel(ModelManager* mgr, const std::string& rootDir)
    : manager(mgr), root(rootDir)
{
    scanFiles();
}

void ModelBrowserPanel::scanFiles()
{
    files.clear();
    std::error_code ec;
    if (!fs::exists(root, ec)) return;
    for (auto& p : fs::recursive_directory_iterator(root, ec)) {
        if (p.is_regular_file()) {
            auto ext = p.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".obj" || ext == ".fbx" || ext == ".dae" || ext == ".gltf" || ext == ".glb") {
                files.push_back(p.path().string());
            }
        }
    }
    std::sort(files.begin(), files.end());
}

bool ModelBrowserPanel::exists(const std::string& path) const
{
    std::error_code ec; return fs::exists(path, ec);
}

void ModelBrowserPanel::draw(bool* open)
{
    if (!open || !(*open)) return;
    if (ImGui::Begin("Models", open)) {
        if (ImGui::Button("Rescan")) { scanFiles(); }
        ImGui::SameLine();
        ImGui::InputText("Filter", &filter);

        ImGui::Separator();
        ImGui::BeginChild("files", ImVec2(0, 300), true);
        int idx = 0;
        for (auto& f : files) {
            if (!filter.empty()) {
                std::string lowf = f; std::transform(lowf.begin(), lowf.end(), lowf.begin(), ::tolower);
                std::string lowq = filter; std::transform(lowq.begin(), lowq.end(), lowq.begin(), ::tolower);
                if (lowf.find(lowq) == std::string::npos) { idx++; continue; }
            }
            bool sel = (idx == selected);
            if (ImGui::Selectable(f.c_str(), sel)) selected = idx;
            idx++;
        }
        ImGui::EndChild();

        bool canLoad = (selected >= 0 && selected < (int)files.size());
        if (ImGui::Button("OK") && canLoad) {
            if (manager) manager->addModel(files[selected]);
        }
        ImGui::SameLine();
        if (ImGui::Button("Close")) {
            *open = false;
        }
    }
    ImGui::End();
}
