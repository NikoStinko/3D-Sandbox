#include "CustomButtonsPanel.h"
#include "EditorState.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

CustomButtonsPanel::CustomButtonsPanel(EditorState* editorState)
    : editor(editorState)
{
}

void CustomButtonsPanel::draw(bool* open)
{
    if (!open || !(*open)) return;
    if (ImGui::Begin("Custom", open)) {
        if (editor) {
            ImGui::Checkbox("Show grid", &editor->gridVisible);
            ImGui::Checkbox("Highlight objects", &editor->highlightObjects);
            ImGui::ColorEdit3("Highlight color", glm::value_ptr(editor->highlightColor));
        } else {
            ImGui::TextDisabled("Editor state unavailable");
        }
    }
    ImGui::End();
}
