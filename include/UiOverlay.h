#ifndef UI_OVERLAY_H
#define UI_OVERLAY_H

#include <string>
#include <memory>
#include <functional>
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

private:
    GLFWwindow* window = nullptr;
    bool visible = false;
    Camera* camera = nullptr;
    SceneState* scene = nullptr;
    EditorState* editor = nullptr;
    std::unique_ptr<ModelBrowserPanel> browser;
    std::unique_ptr<CustomButtonsPanel> buttons;
    std::unique_ptr<ScenePanel> scenePanel;
    std::unique_ptr<MapPanel> mapPanel;
};

#endif // UI_OVERLAY_H
