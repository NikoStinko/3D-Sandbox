#include "CustomButtonsPanel.h"
#include <imgui.h>

void CustomButtonsPanel::draw(bool* open)
{
    if (!open || !(*open)) return;
    if (ImGui::Begin("Custom", open)) {
        ImGui::Text("Ajoutez ici vos boutons personnalisés.");
        // Exemple de boutons placeholder:
        if (ImGui::Button("Wireframe ON/OFF")) {
            static bool wire = false;
            wire = !wire;
            ImGui::Text("Wireframe: %s", wire ? "ON" : "OFF");
            // L'appel OpenGL réel sera géré ailleurs si besoin
        }
    }
    ImGui::End();
}
