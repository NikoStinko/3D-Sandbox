#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

struct GLFWwindow;
class ModelManager;
class Camera;
class UiOverlay;
class EditorState;

class InputHandler {
public:
    void update(GLFWwindow* window, EditorState& editorState, ModelManager& modelManager, Camera& camera, UiOverlay& overlay, bool& overlayOpenedForPause);
};

#endif // INPUT_HANDLER_H