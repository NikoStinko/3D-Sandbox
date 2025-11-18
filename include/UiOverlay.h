#ifndef UI_OVERLAY_H
#define UI_OVERLAY_H

#include <string>
#include <memory>
struct GLFWwindow;
class Camera;

class ModelManager;
class ModelBrowserPanel;
class CustomButtonsPanel;

class UiOverlay {
public:
    UiOverlay();
    ~UiOverlay();

    void init(GLFWwindow* window, const std::string& modelsRoot, ModelManager* mgr, Camera* cam);
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
    std::unique_ptr<ModelBrowserPanel> browser;
    std::unique_ptr<CustomButtonsPanel> buttons;
};

#endif // UI_OVERLAY_H
