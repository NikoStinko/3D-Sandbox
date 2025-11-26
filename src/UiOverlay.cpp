#include "UiOverlay.h"
#include "ModelBrowserPanel.h"
#include "CustomButtonsPanel.h"
#include "ScenePanel.h"
#include "SceneState.h"
#include "MapPanel.h"
#include "EditorState.h"
#include "ModelManager.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>
#include "Camera.h"

UiOverlay::UiOverlay() {}
UiOverlay::~UiOverlay() { shutdown(); }

void UiOverlay::init(GLFWwindow* win,
                     const std::string& modelsRoot,
                     const std::string& mapsRoot,
                     ModelManager* mgr,
                     Camera* cam,
                     SceneState* sceneState,
                     EditorState* editorState,
                     const std::function<bool(const std::string&)>& saveCb,
                     const std::function<bool(const std::string&)>& loadCb,
                     const std::function<bool(const std::string&)>& newCb)
{
    window = win;
    camera = cam;
    scene = sceneState;
    editor = editorState;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    // GLSL version string depending on your context; 130 works for GL 3.0+, 330 for core
    ImGui_ImplOpenGL3_Init("#version 130");

    models = mgr;
    browser = std::make_unique<ModelBrowserPanel>(mgr, modelsRoot);
    buttons = std::make_unique<CustomButtonsPanel>(editorState);
    scenePanel = std::make_unique<ScenePanel>(sceneState);
    mapPanel = std::make_unique<MapPanel>(mapsRoot, saveCb, loadCb, newCb, editorState);

    // Assurer qu'aucun panneau n'est visible par défaut
    if (browser) browser->setVisible(false);
    if (buttons) buttons->setVisible(false);
    if (scenePanel) scenePanel->setVisible(false);
    if (mapPanel) mapPanel->setVisible(false);
}

void UiOverlay::shutdown()
{
    if (!window) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    window = nullptr;
}

void UiOverlay::toggleVisible()
{
    visible = !visible;
    if (window) {
        glfwSetInputMode(window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }
}

void UiOverlay::beginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UiOverlay::draw()
{
    if (!visible) return;

    // Dessiner le menu pause si actif
    static MenuState prevMenuState = MenuState::None;
    if (editor && editor->menuState == MenuState::Pause) {
        // À l'ouverture du menu pause, masquer tous les panneaux pour éviter l'empilement
        if (prevMenuState != MenuState::Pause) {
            hideAllPanels();
        }
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("PauseMenu", nullptr, 
                    ImGuiWindowFlags_NoTitleBar | 
                    ImGuiWindowFlags_NoResize | 
                    ImGuiWindowFlags_NoMove |
                    ImGuiWindowFlags_NoScrollbar |
                    ImGuiWindowFlags_NoSavedSettings |
                    ImGuiWindowFlags_NoBringToFrontOnFocus);
        
        // Fond semi-transparent
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(
            ImVec2(0, 0), 
            ImGui::GetIO().DisplaySize, 
            IM_COL32(0, 0, 0, 180)
        );
        
        // Titre
        ImGui::SetCursorPos(ImVec2(
            (ImGui::GetWindowWidth() - ImGui::CalcTextSize("PAUSE").x) * 0.5f,
            ImGui::GetWindowHeight() * 0.2f
        ));
        ImGui::TextColored(ImVec4(1, 1, 1, 1), "PAUSE");
        
        // Bouton pour retourner au jeu
        ImGui::SetCursorPos(ImVec2(
            (ImGui::GetWindowWidth() - 200) * 0.5f,
            ImGui::GetWindowHeight() * 0.4f
        ));
        if (ImGui::Button("Resume", ImVec2(200, 40))) {
            editor->menuState = MenuState::None;
            // Fermer l'overlay uniquement si aucun panneau n'est ouvert
            bool anyPanelOpen = (browser && browser->isVisible()) || (buttons && buttons->isVisible()) || (scenePanel && scenePanel->isVisible()) || (mapPanel && mapPanel->isVisible());
            if (!anyPanelOpen && visible) toggleVisible();
        }
        
        // Bouton pour ouvrir la sélection de map
        ImGui::SetCursorPos(ImVec2(
            (ImGui::GetWindowWidth() - 200) * 0.5f,
            ImGui::GetWindowHeight() * 0.5f
        ));
        if (ImGui::Button("Maps", ImVec2(200, 40))) {
            // Afficher uniquement le panel des maps
            if (mapPanel) {
                hideAllPanels();
                mapPanel->setVisible(true);
            }
            editor->menuState = MenuState::None;
        }
        
        // Bouton pour quitter
        ImGui::SetCursorPos(ImVec2(
            (ImGui::GetWindowWidth() - 200) * 0.5f,
            ImGui::GetWindowHeight() * 0.6f
        ));
        if (ImGui::Button("Quitter", ImVec2(200, 40))) {
            glfwSetWindowShouldClose(window, true);
        }
        
        ImGui::End();
    }
    prevMenuState = editor ? editor->menuState : MenuState::None;
    
    // Dessiner le menu radial si actif et dans une map
    if (editor && editor->menuState == MenuState::Radial && editor->isInMap) {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("RadialMenu", nullptr, 
                    ImGuiWindowFlags_NoTitleBar | 
                    ImGuiWindowFlags_NoResize | 
                    ImGuiWindowFlags_NoMove |
                    ImGuiWindowFlags_NoScrollbar |
                    ImGuiWindowFlags_NoBackground |
                    ImGuiWindowFlags_NoSavedSettings |
                    ImGuiWindowFlags_NoBringToFrontOnFocus);
        
        ImVec2 displaySize = ImGui::GetIO().DisplaySize;
        ImVec2 center(displaySize.x * 0.5f, displaySize.y * 0.5f);
        float radius = ImGui::GetIO().DisplaySize.y * 0.3f;
        
        // Variables pour suiver les survols
        static bool hovered[4] = {false};
        
        // Dessiner les boutons du menu radial (4 sections)
        const float PI = 3.14159265358979323846f;
        DrawRadialButton("Infos/Logs", center, radius, 0.0f, PI * 0.5f, hovered[0]);
        DrawRadialButton("Import", center, radius, PI * 0.5f, PI, hovered[1]);
        DrawRadialButton("Scène", center, radius, PI, PI * 1.5f, hovered[2]);
        DrawRadialButton("Personnalisé", center, radius, PI * 1.5f, PI * 2.0f, hovered[3]);
        
        // Gérer les clics
        if (ImGui::IsMouseClicked(0)) {
            if (hovered[0]) {
                editor->activeRadialItem = RadialMenuItem::InfoLogs;
                // Afficher le panneau d'infos/logs
                if (scenePanel) scenePanel->setVisible(true);
            } else if (hovered[1]) {
                editor->activeRadialItem = RadialMenuItem::ImportModels;
                // Afficher le navigateur de modèles
                if (browser) browser->setVisible(true);
            } else if (hovered[2]) {
                editor->activeRadialItem = RadialMenuItem::SceneSettings;
                // Afficher les paramètres de scène
                if (scenePanel) scenePanel->setVisible(true);
            } else if (hovered[3]) {
                editor->activeRadialItem = RadialMenuItem::CustomMenu;
                // Afficher le menu personnalisé
                if (buttons) buttons->setVisible(true);
            }
            
            // Fermer le menu radial après sélection
            editor->menuState = MenuState::None;
        }
        
        ImGui::End();
    }

    // Menu contextuel (clic droit) pour l'objet sélectionné
    if (editor && editor->selectedObject.has_value()) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup("ObjectContext");
            contextIndex = editor->selectedObject->modelIndex;
        }
        if (ImGui::BeginPopup("ObjectContext")) {
            if (ImGui::MenuItem("Resize")) {
                showResizePopup = true;
                tmpScale = editor->selectedObject->scale;
            }
            if (ImGui::MenuItem("Rotate")) {
                showRotatePopup = true;
                tmpRotation = editor->selectedObject->rotation;
            }
            ImGui::EndPopup();
        }
        if (showResizePopup) {
            ImGui::SetNextWindowSize(ImVec2(320, 160), ImGuiCond_Appearing);
            if (ImGui::Begin("Resize Object", &showResizePopup, ImGuiWindowFlags_NoSavedSettings)) {
                ImGui::Text("Scale");
                ImGui::DragFloat3("##scale", &tmpScale.x, 0.01f, 0.01f, 100.0f);
                if (ImGui::Button("Apply")) {
                    if (models && contextIndex < models->getModelCount()) {
                        auto sel = *editor->selectedObject;
                        models->updateModel(contextIndex, sel.position, sel.rotation, tmpScale, sel.opacity);
                        editor->selectedObject->scale = tmpScale;
                    }
                    showResizePopup = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) { showResizePopup = false; }
                ImGui::End();
            }
        }
        if (showRotatePopup) {
            ImGui::SetNextWindowSize(ImVec2(320, 160), ImGuiCond_Appearing);
            if (ImGui::Begin("Rotate Object", &showRotatePopup, ImGuiWindowFlags_NoSavedSettings)) {
                ImGui::Text("Rotation (deg)");
                ImGui::DragFloat3("##rot", &tmpRotation.x, 0.5f, -360.0f, 360.0f);
                if (ImGui::Button("Apply")) {
                    if (models && contextIndex < models->getModelCount()) {
                        auto sel = *editor->selectedObject;
                        models->updateModel(contextIndex, sel.position, tmpRotation, sel.scale, sel.opacity);
                        editor->selectedObject->rotation = tmpRotation;
                    }
                    showRotatePopup = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) { showRotatePopup = false; }
                ImGui::End();
            }
        }
    }

    // Dessiner les panneaux d'interface utilisateur standards
    bool open = true;
    if (browser && browser->isVisible()) browser->draw(&open);
    if (buttons && buttons->isVisible()) buttons->draw(&open);
    if (scenePanel && scenePanel->isVisible()) scenePanel->draw(&open);
    if (mapPanel && mapPanel->isVisible()) mapPanel->draw(&open);

    // Afficher le message de statut s'il est actif
    if (editor && !editor->statusMessage.empty() && editor->statusMessageTime > 0.0f) {
        ImGui::SetNextWindowPos(ImVec2(10, 10));
        ImGui::SetNextWindowBgAlpha(0.35f);
        if (ImGui::Begin("Status", nullptr, 
                        ImGuiWindowFlags_NoTitleBar | 
                        ImGuiWindowFlags_NoResize | 
                        ImGuiWindowFlags_NoSavedSettings |
                        ImGuiWindowFlags_NoFocusOnAppearing |
                        ImGuiWindowFlags_NoNav)) {
            ImGui::Text("%s", editor->statusMessage.c_str());
        }
        ImGui::End();
    }
    
    // Afficher les FPS en haut à droite
    if (editor) {
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 100, 10));
        ImGui::SetNextWindowBgAlpha(0.35f);
        if (ImGui::Begin("FPS", nullptr, 
                        ImGuiWindowFlags_NoTitleBar | 
                        ImGuiWindowFlags_NoResize | 
                        ImGuiWindowFlags_NoSavedSettings |
                        ImGuiWindowFlags_NoFocusOnAppearing |
                        ImGuiWindowFlags_NoNav)) {
            ImGui::Text("FPS: %.1f", editor->fps);
        }
        ImGui::End();
    }
}

void UiOverlay::endFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UiOverlay::hideAllPanels()
{
    if (browser) browser->setVisible(false);
    if (buttons) buttons->setVisible(false);
    if (scenePanel) scenePanel->setVisible(false);
    if (mapPanel) mapPanel->setVisible(false);
}

void UiOverlay::showOnlyMapPanel()
{
    if (!visible) toggleVisible();
    hideAllPanels();
    if (mapPanel) mapPanel->setVisible(true);
}

void UiOverlay::showOnlyModelBrowser()
{
    if (!visible) toggleVisible();
    hideAllPanels();
    if (browser) browser->setVisible(true);
}

void UiOverlay::showOnlyScenePanel()
{
    if (!visible) toggleVisible();
    hideAllPanels();
    if (scenePanel) scenePanel->setVisible(true);
}

void UiOverlay::showOnlyCustomButtons()
{
    if (!visible) toggleVisible();
    hideAllPanels();
    if (buttons) buttons->setVisible(true);
}

void UiOverlay::DrawRadialButton(const char* label, const ImVec2& center, float radius, float angleStart, float angleEnd, bool& hovered) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const ImU32 col = hovered ? IM_COL32(200, 200, 100, 200) : IM_COL32(100, 100, 100, 200);
    const ImU32 col_hovered = IM_COL32(200, 200, 100, 255);
    
    // Dessiner l'arc du bouton
    draw_list->PathArcTo(center, radius, angleStart, angleEnd);
    draw_list->PathLineTo(center);
    draw_list->PathFillConvex(hovered ? col_hovered : col);
    
    // Dessiner le texte au milieu de l'arc
    float midAngle = (angleStart + angleEnd) * 0.5f;
    ImVec2 textPos(
        center.x + cosf(midAngle) * radius * 0.6f - ImGui::CalcTextSize(label).x * 0.5f,
        center.y + sinf(midAngle) * radius * 0.6f - ImGui::GetTextLineHeight() * 0.5f
    );
    draw_list->AddText(textPos, IM_COL32(255, 255, 255, 255), label);
    
    // Vérifier si la souris est sur le bouton
    ImVec2 mousePos = ImGui::GetIO().MousePos;
    float distSq = (mousePos.x - center.x) * (mousePos.x - center.x) + 
                  (mousePos.y - center.y) * (mousePos.y - center.y);
    float angle = atan2f(mousePos.y - center.y, mousePos.x - center.x);
    if (angle < 0) angle += 2 * 3.14159265358979323846f;
    
    hovered = (distSq >= (radius * 0.5f) * (radius * 0.5f)) && 
              (distSq <= radius * radius) && 
              (angle >= angleStart) && 
              (angle <= angleEnd);
}
