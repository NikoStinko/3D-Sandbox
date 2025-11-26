#include "MenuRenderer.h"
#include <imgui.h>
#include <cmath>
#include <ostream>
#include <iostream>

void MenuRenderer::drawRadialMenu(const EditorState& editorState, const glm::vec2& center, float radius) {
    // Dessiner le fond du menu radial
    ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(center.x, center.y), radius, IM_COL32(40, 40, 40, 200));
    ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(center.x, center.y), radius, IM_COL32(255, 255, 255, 255), 0, 2.0f);
    
    // Dessiner les boutons
    const float angleStep = 2.0f * M_PI / 4.0f; // 4 boutons
    const float startAngle = -M_PI / 2.0f; // Commencer en haut
    
    drawRadialButton(center, radius, startAngle + 0 * angleStep, "Infos", editorState.activeRadialItem == RadialMenuItem::InfoLogs);
    drawRadialButton(center, radius, startAngle + 1 * angleStep, "Import", editorState.activeRadialItem == RadialMenuItem::ImportModels);
    drawRadialButton(center, radius, startAngle + 2 * angleStep, "Scène", editorState.activeRadialItem == RadialMenuItem::SceneSettings);
    drawRadialButton(center, radius, startAngle + 3 * angleStep, "Custom", editorState.activeRadialItem == RadialMenuItem::CustomMenu);
}

void MenuRenderer::drawPauseMenu(const EditorState& editorState) {
    // Obtenir la taille de la fenêtre
    ImGuiIO& io = ImGui::GetIO();
    glm::vec2 windowSize(io.DisplaySize.x, io.DisplaySize.y);
    
    // Dimensions du menu
    const glm::vec2 menuSize(300, 200);
    const glm::vec2 menuPos = (windowSize - menuSize) * 0.5f;
    
    // Dessiner le fond
    ImGui::GetBackgroundDrawList()->AddRectFilled(
        ImVec2(menuPos.x, menuPos.y), 
        ImVec2(menuPos.x + menuSize.x, menuPos.y + menuSize.y),
        IM_COL32(40, 40, 40, 230)
    );
    ImGui::GetBackgroundDrawList()->AddRect(
        ImVec2(menuPos.x, menuPos.y), 
        ImVec2(menuPos.x + menuSize.x, menuPos.y + menuSize.y),
        IM_COL32(255, 255, 255, 255), 0.0f, 0, 2.0f
    );
    
    // Dessiner les boutons
    const glm::vec2 buttonSize(250, 40);
    const float buttonSpacing = 10.0f;
    const float startY = menuPos.y + 20.0f;
    
    drawPauseButton(glm::vec2(menuPos.x + 25, startY), buttonSize, "Liste des Maps", editorState.activePauseItem == PauseMenuItem::MapList);
    drawPauseButton(glm::vec2(menuPos.x + 25, startY + (buttonSize.y + buttonSpacing) * 1), buttonSize, "Menu Radial", editorState.activePauseItem == PauseMenuItem::RadialMenu);
    drawPauseButton(glm::vec2(menuPos.x + 25, startY + (buttonSize.y + buttonSpacing) * 2), buttonSize, "Quitter", editorState.activePauseItem == PauseMenuItem::Quit);
    drawPauseButton(glm::vec2(menuPos.x + 25, startY + (buttonSize.y + buttonSpacing) * 3), buttonSize, "Reprendre", editorState.activePauseItem == PauseMenuItem::Resume);
}

void MenuRenderer::drawRadialButton(const glm::vec2& center, float radius, float angle, const std::string& label, bool selected) {
    const float buttonRadius = radius * 0.3f;
    const float buttonDistance = radius * 0.7f;
    
    glm::vec2 buttonPos = getRadialPosition(center, buttonDistance, angle);
    
    // Couleur du bouton
    ImU32 color = selected ? IM_COL32(100, 150, 255, 255) : IM_COL32(80, 80, 80, 255);
    ImU32 textColor = IM_COL32(255, 255, 255, 255);
    
    // Dessiner le bouton
    ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(buttonPos.x, buttonPos.y), buttonRadius, color);
    ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(buttonPos.x, buttonPos.y), buttonRadius, IM_COL32(255, 255, 255, 255), 0, 2.0f);
    
    // Dessiner le texte
    ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
    ImGui::GetBackgroundDrawList()->AddText(
        ImVec2(buttonPos.x - textSize.x * 0.5f, buttonPos.y - textSize.y * 0.5f),
        textColor,
        label.c_str()
    );
}

void MenuRenderer::drawPauseButton(const glm::vec2& position, const glm::vec2& size, const std::string& label, bool selected) {
    // Couleur du bouton
    ImU32 color = selected ? IM_COL32(100, 150, 255, 255) : IM_COL32(60, 60, 60, 255);
    ImU32 textColor = IM_COL32(255, 255, 255, 255);
    
    // Dessiner le bouton
    ImGui::GetBackgroundDrawList()->AddRectFilled(
        ImVec2(position.x, position.y),
        ImVec2(position.x + size.x, position.y + size.y),
        color
    );
    ImGui::GetBackgroundDrawList()->AddRect(
        ImVec2(position.x, position.y),
        ImVec2(position.x + size.x, position.y + size.y),
        IM_COL32(255, 255, 255, 255), 0.0f, 0, 2.0f
    );
    
    // Dessiner le texte
    ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
    ImGui::GetBackgroundDrawList()->AddText(
        ImVec2(position.x + (size.x - textSize.x) * 0.5f, position.y + (size.y - textSize.y) * 0.5f),
        textColor,
        label.c_str()
    );
}

glm::vec2 MenuRenderer::getRadialPosition(const glm::vec2& center, float radius, float angle) {
    return glm::vec2(
        center.x + radius * cos(angle),
        center.y + radius * sin(angle)
    );
}

RadialMenuItem MenuRenderer::handleRadialMenuClick(const EditorState& editorState, const glm::vec2& clickPos, const glm::vec2& center, float radius) {
    // Calculer l'angle du clic par rapport au centre
    glm::vec2 diff = clickPos - center;
    float distance = glm::length(diff);
    
    // Vérifier si le clic est dans le rayon du menu
    if (distance > radius * 0.4f && distance < radius * 0.9f) {
        float angle = atan2(diff.y, diff.x);
        // Normaliser l'angle entre 0 et 2π
        if (angle < 0) angle += 2.0f * M_PI;
        
        return getRadialItemFromAngle(angle);
    }
    
    return RadialMenuItem::None;
}

PauseMenuItem MenuRenderer::handlePauseMenuClick(const EditorState& editorState, const glm::vec2& clickPos) {
    // Obtenir la taille de la fenêtre
    ImGuiIO& io = ImGui::GetIO();
    glm::vec2 windowSize(io.DisplaySize.x, io.DisplaySize.y);
    
    // Dimensions du menu
    const glm::vec2 menuSize(300, 200);
    const glm::vec2 menuPos = (windowSize - menuSize) * 0.5f;
    
    // Dimensions des boutons
    const glm::vec2 buttonSize(250, 40);
    const float buttonSpacing = 10.0f;
    const float startY = menuPos.y + 20.0f;
    const float startX = menuPos.x + 25.0f;
    
    // Vérifier chaque bouton
    for (int i = 0; i < 4; i++) {
        glm::vec2 buttonPos(startX, startY + (buttonSize.y + buttonSpacing) * i);
        
        if (clickPos.x >= buttonPos.x && clickPos.x <= buttonPos.x + buttonSize.x &&
            clickPos.y >= buttonPos.y && clickPos.y <= buttonPos.y + buttonSize.y) {
            return getPauseItemFromPosition(buttonPos);
        }
    }
    
    return PauseMenuItem::None;
}

RadialMenuItem MenuRenderer::getRadialItemFromAngle(float angle) {
    const float angleStep = 2.0f * M_PI / 4.0f;
    const float startAngle = -M_PI / 2.0f;
    
    // Normaliser l'angle
    float normalizedAngle = angle - startAngle;
    if (normalizedAngle < 0) normalizedAngle += 2.0f * M_PI;
    
    int buttonIndex = static_cast<int>(normalizedAngle / angleStep);
    
    switch (buttonIndex) {
        case 0: return RadialMenuItem::InfoLogs;
        case 1: return RadialMenuItem::ImportModels;
        case 2: return RadialMenuItem::SceneSettings;
        case 3: return RadialMenuItem::CustomMenu;
        default: return RadialMenuItem::None;
    }
}

PauseMenuItem MenuRenderer::getPauseItemFromPosition(const glm::vec2& position) {
    // Obtenir la taille de la fenêtre
    ImGuiIO& io = ImGui::GetIO();
    glm::vec2 windowSize(io.DisplaySize.x, io.DisplaySize.y);
    
    // Dimensions du menu
    const glm::vec2 menuSize(300, 200);
    const glm::vec2 menuPos = (windowSize - menuSize) * 0.5f;
    
    // Dimensions des boutons
    const glm::vec2 buttonSize(250, 40);
    const float buttonSpacing = 10.0f;
    const float startY = menuPos.y + 20.0f;
    
    // Calculer l'index du bouton
    float relativeY = position.y - startY;
    int buttonIndex = static_cast<int>(relativeY / (buttonSize.y + buttonSpacing));
    
    switch (buttonIndex) {
        case 0: return PauseMenuItem::MapList;
        case 1: return PauseMenuItem::RadialMenu;
        case 2: return PauseMenuItem::Quit;
        case 3: return PauseMenuItem::Resume;
        default: return PauseMenuItem::None;
    }
}