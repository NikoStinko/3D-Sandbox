#ifndef UI_OVERLAY_H
#define UI_OVERLAY_H

#include <string>
#include <memory>
#include <functional>

#include <imgui.h>
#include <glm/glm.hpp>
struct GLFWwindow;
class Camera;
class SceneState;
class EditorState;

class ModelManager;
class ModelBrowserPanel;
class CustomButtonsPanel;
class ScenePanel;
class MapPanel;

class UiOverlay {
public:
    UiOverlay();
    ~UiOverlay();

    void init(GLFWwindow* window,
              const std::string& modelsRoot,
              const std::string& mapsRoot,
              ModelManager* mgr,
              Camera* cam,
              SceneState* sceneState,
              EditorState* editorState,
              const std::function<bool(const std::string&)>& saveCb,
              const std::function<bool(const std::string&)>& loadCb,
              const std::function<bool(const std::string&)>& newCb);
    void shutdown();

    void toggleVisible();
    bool isVisible() const { return visible; }

    void beginFrame();
    void draw();
    void endFrame();

    // Utilitaires de visibilité des panneaux
    void hideAllPanels();
    void showOnlyMapPanel();
    void showOnlyModelBrowser();
    void showOnlyScenePanel();
    void showOnlyCustomButtons();

private:
    // Fonction utilitaire pour dessiner un bouton de menu radial
    void DrawRadialButton(const char* label, const ImVec2& center, float radius, float angleStart, float angleEnd, bool& hovered);
    
    GLFWwindow* window = nullptr;
    bool visible = false;
    Camera* camera = nullptr;
    SceneState* scene = nullptr;
    EditorState* editor = nullptr;
    ModelManager* models = nullptr;

    // État du menu contextuel (clic droit)
    bool showResizePopup = false;
    bool showRotatePopup = false;
    size_t contextIndex = 0;
    glm::vec3 tmpScale = glm::vec3(1.0f);
    glm::vec3 tmpRotation = glm::vec3(0.0f);

    std::unique_ptr<ModelBrowserPanel> browser;
    std::unique_ptr<CustomButtonsPanel> buttons;
    std::unique_ptr<ScenePanel> scenePanel;
    std::unique_ptr<MapPanel> mapPanel;
};

#endif // UI_OVERLAY_H
