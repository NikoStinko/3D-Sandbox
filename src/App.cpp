#include "App.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <filesystem>

App* App::instance = nullptr;

bool App::init(unsigned int width, unsigned int height, const char* title) {
    instance = this;

    if (!window.create(width, height, title)) return false;

    // GLEW init
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);

    // Callbacks
    glfwSetFramebufferSizeCallback(window.getGLFW(), framebuffer_size_callback);
    glfwSetCursorPosCallback(window.getGLFW(), mouse_callback);
    glfwSetScrollCallback(window.getGLFW(), scroll_callback);

    // Shaders
    ourShader = Shader("../shaders/model.vs", "../shaders/model.fs");

    // Grille
    grid = Grid();

    // États
    editorState = EditorState();
    sceneState = SceneState();

    // Scene (pour map)
    scene = std::make_unique<Scene>(editorState, manager, sceneState);

    // UI
    overlay.init(window.getGLFW(), "../resources/models", "../resources/maps", &manager, &camera, &sceneState, &editorState,
        [this](const std::string& name) { return scene->saveMap(name, "../resources/maps"); },
        [this](const std::string& name) { return scene->loadMap(name, "../resources/maps"); },
        [this](const std::string& name) { return scene->createMap(name, "../resources/maps"); });

    sandboxUI = SandBoxUI(&manager, &editorState, &camera);

    return true;
}

int App::run() {
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    while (!window.shouldClose()) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();

        inputHandler.update(window.getGLFW(), editorState, manager, camera, overlay, overlayOpenedForPause);

        editorState.update(deltaTime);

        overlay.beginFrame();
        sandboxUI.draw(window.getGLFW());

        // Rendu
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderer.render(ourShader, manager, grid, camera, deltaTime, editorState);

        overlay.draw();
        overlay.endFrame();

        window.swapBuffers();
    }

    return 0;
}

void App::shutdown() {
    overlay.shutdown();
    window.destroy();
}

void App::framebuffer_size_callback(GLFWwindow* w, int width, int height) {
    if (instance) instance->onResize(width, height);
}
void App::mouse_callback(GLFWwindow* w, double xpos, double ypos) {
    if (instance) instance->onMouseMove(xpos, ypos);
}
void App::scroll_callback(GLFWwindow* w, double xoffset, double yoffset) {
    if (instance) instance->onScroll(xoffset, yoffset);
}

void App::onResize(int width, int height) {
    glViewport(0, 0, width, height);
}
void App::onMouseMove(double xpos, double ypos) {
    if (glfwGetInputMode(window.getGLFW(), GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {

        if (overlay.isVisible()) return;

        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
            return;       // on ignore la première frame
        }

        double xoffset = xpos - lastX;
        double yoffset = (ypos - lastY) / 2.0;

        lastX = xpos;
        lastY = ypos;

        camera.ProcessMouseMovement(xoffset, yoffset);
    } else {
        // Quand la souris n'est plus capturée, on prépare la réinitialisation
        firstMouse = true;
    }
}
void App::onScroll(double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}