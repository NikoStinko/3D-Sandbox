#include "InputHandler.h"
#include "ModelManager.h"
#include "Camera.h"
#include "UiOverlay.h"
#include "EditorState.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void InputHandler::update(GLFWwindow* window, EditorState& editorState, ModelManager& modelManager, Camera& camera, UiOverlay& overlay, bool& overlayOpenedForPause) {
    static bool escapeWasPressed = false;
    static bool tabWasPressed = false;
    static bool leftMouseWasPressed = false;
    static float lastFrame = 0.0f;

    float currentFrame = glfwGetTime();
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    bool leftMouseIsPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    leftMouseWasPressed = leftMouseIsPressed;

    // ESC
    bool escapeIsPressed = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
    if (escapeIsPressed && !escapeWasPressed) {
        if (editorState.menuState == MenuState::Pause) {
            editorState.menuState = MenuState::None;
            editorState.activePauseItem = PauseMenuItem::None;
            if (overlayOpenedForPause && overlay.isVisible()) { overlay.toggleVisible(); overlayOpenedForPause = false; }
        } else if (editorState.menuState == MenuState::Radial) {
            editorState.menuState = MenuState::Pause;
            editorState.activePauseItem = PauseMenuItem::None;
            if (!overlay.isVisible()) { overlay.toggleVisible(); overlayOpenedForPause = true; }
        } else if (overlay.isVisible()) {
            overlay.hideAllPanels();
            overlay.toggleVisible();
            overlayOpenedForPause = false;
        } else {
            editorState.menuState = MenuState::Pause;
            editorState.activePauseItem = PauseMenuItem::Resume;
            overlay.hideAllPanels();
            if (!overlay.isVisible()) { overlay.toggleVisible(); overlayOpenedForPause = true; }
        }
    }
    escapeWasPressed = escapeIsPressed;

    // TAB
    bool tabIsPressed = glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS;
    if (tabIsPressed && !tabWasPressed && editorState.menuState != MenuState::Pause) {
        if (editorState.menuState == MenuState::Radial) {
            editorState.menuState = MenuState::None;
            editorState.activeRadialItem = RadialMenuItem::None;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            editorState.menuState = MenuState::Radial;
            editorState.activeRadialItem = RadialMenuItem::None;
            if (overlayOpenedForPause && overlay.isVisible()) { overlay.toggleVisible(); overlayOpenedForPause = false; }
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    tabWasPressed = tabIsPressed;

    // Curseur
    if (editorState.menuState != MenuState::None || overlay.isVisible()) {
        if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_NORMAL) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    } else {
        if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }

    // F1 : attacher/détacher la souris
    static bool f1WasPressed = false;
    bool f1IsPressed = glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS;
    if (f1IsPressed && !f1WasPressed) {
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
    f1WasPressed = f1IsPressed;

    // Mouvements caméra (si curseur capturé)
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);
    }
}