#include "ScenePanel.h"
#include "SceneState.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

ScenePanel::ScenePanel(SceneState* state)
    : scene(state)
{
}

void ScenePanel::draw(bool* open)
{
    if (!scene) return;
    if (open && !(*open)) return;

    if (ImGui::Begin("Scene", open)) {
        auto& light = scene->light();
        auto& env = scene->environment();

        if (ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat3("Direction", glm::value_ptr(light.direction), 0.01f, -1.0f, 1.0f);
            ImGui::SliderFloat("Intensity", &light.intensity, 0.0f, 5.0f);
            ImGui::ColorEdit3("Ambient", glm::value_ptr(light.ambient));
            ImGui::ColorEdit3("Diffuse", glm::value_ptr(light.diffuse));
            ImGui::ColorEdit3("Specular", glm::value_ptr(light.specular));
        }

        if (ImGui::CollapsingHeader("Environment", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::ColorEdit3("Sky Color", glm::value_ptr(env.skyColor));
            ImGui::SliderFloat("Ambient Boost", &env.ambientBoost, 0.0f, 5.0f);
        }
    }
    ImGui::End();
}
