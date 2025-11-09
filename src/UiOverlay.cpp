#include "UiOverlay.h"
#include "ModelBrowserPanel.h"
#include "CustomButtonsPanel.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

UiOverlay::UiOverlay() {}
UiOverlay::~UiOverlay() { shutdown(); }

void UiOverlay::init(GLFWwindow* win, const std::string& modelsRoot, ModelManager* mgr)
{
    window = win;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    // GLSL version string depending on your context; 130 works for GL 3.0+, 330 for core
    ImGui_ImplOpenGL3_Init("#version 130");

    browser = std::make_unique<ModelBrowserPanel>(mgr, modelsRoot);
    buttons = std::make_unique<CustomButtonsPanel>();
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
    if (browser) browser->draw(&openBrowser);
    if (buttons) buttons->draw(&openButtons);
}

void UiOverlay::endFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
