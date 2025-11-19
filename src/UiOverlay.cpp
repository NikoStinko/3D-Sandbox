#include "UiOverlay.h"
#include "ModelBrowserPanel.h"
#include "CustomButtonsPanel.h"
#include "ScenePanel.h"
#include "SceneState.h"
#include "MapPanel.h"
#include "EditorState.h"

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

    browser = std::make_unique<ModelBrowserPanel>(mgr, modelsRoot);
    buttons = std::make_unique<CustomButtonsPanel>(editorState);
    scenePanel = std::make_unique<ScenePanel>(sceneState);
    mapPanel = std::make_unique<MapPanel>(mapsRoot, saveCb, loadCb, newCb, editorState);
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
    bool openBrowser = true;
    bool openButtons = true;
    bool openScene = true;
    bool openMaps = true;
    if (browser) browser->draw(&openBrowser);
    if (buttons) buttons->draw(&openButtons);
    if (scenePanel) scenePanel->draw(&openScene);
    if (mapPanel) mapPanel->draw(&openMaps);

    // Camera info overlay
    if (camera) {
        ImGui::Begin("Info");
        ImGui::Text("Camera: x=%.2f y=%.2f z=%.2f", camera->Position.x, camera->Position.y, camera->Position.z);
        if (editor) {
            ImGui::Text("FPS: %.1f", editor->fps);
            if (!editor->statusMessage.empty()) {
                ImGui::Separator();
                ImGui::TextWrapped("%s", editor->statusMessage.c_str());
            }
        }
        ImGui::End();
    }
}

void UiOverlay::endFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
