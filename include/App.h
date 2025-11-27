#ifndef APP_H
#define APP_H

#include "Window.h"
#include "InputHandler.h"
#include "Renderer.h"
#include "Scene.h"
#include "SandBoxUI.h"
#include "UiOverlay.h"
#include "ModelManager.h"
#include "Camera.h"
#include "EditorState.h"
#include "SceneState.h"
#include "Shader.h"
#include "Grid.h"
#include "MenuRenderer.h"
#include <memory>

class App {
public:
    bool init(unsigned int width, unsigned int height, const char* title);
    int run();
    void shutdown();

private:
    Window window;
    InputHandler inputHandler;
    Renderer renderer;
    std::unique_ptr<Scene> scene;
    ModelManager manager;
    Camera camera;
    EditorState editorState;
    SceneState sceneState;
    UiOverlay overlay;
    SandBoxUI sandboxUI;
    Shader ourShader;
    Grid grid;
    bool overlayOpenedForPause = false;

    // Gestion de la souris pour la caméra
    bool firstMouse = true;
    double lastX = 0.0;
    double lastY = 0.0;

    static void framebuffer_size_callback(GLFWwindow* w, int width, int height);
    static void mouse_callback(GLFWwindow* w, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* w, double xoffset, double yoffset);
    static App* instance;

    void onResize(int width, int height);
    void onMouseMove(double xpos, double ypos);
    void onScroll(double xoffset, double yoffset);
};

#endif // APP_H