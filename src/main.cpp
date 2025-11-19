#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "ModelManager.h"
#include "UiOverlay.h"
#include "SceneState.h"
#include "Grid.h"
#include "SceneSerializer.h"
#include "EditorState.h"
#include "SceneData.h"

#include <imgui.h>

#include <iostream>
#include <filesystem>

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);

// Window dimensions
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct AppContext {
    ModelManager* models = nullptr;
    SceneState* scene = nullptr;
    EditorState* editor = nullptr;
    std::string mapsRoot;

    std::string normalized(const std::string& path) const {
        if (path.empty()) return path;
        return std::filesystem::path(path).lexically_normal().string();
    }

    std::string ensureJsonExtension(const std::string& name) const {
        std::filesystem::path p = name;
        if (p.extension() != ".json") {
            p.replace_extension(".json");
        }
        return p.string();
    }

    std::string currentMapPath() const {
        std::string mapName = (editor && !editor->currentMapName.empty()) ? editor->currentMapName : "default";
        std::filesystem::path base = mapsRoot.empty() ? std::filesystem::path(".") : std::filesystem::path(mapsRoot);
        base /= ensureJsonExtension(mapName);
        return base.string();
    }

    bool saveSceneTo(const std::string& path) {
        if (!models || !scene) return false;
        std::string resolved = normalized(path.empty() ? currentMapPath() : path);
        if (resolved.empty()) return false;

        SceneSnapshot snapshot;
        snapshot.light = scene->light();
        snapshot.environment = scene->environment();
        snapshot.models = models->serializeInstances();

        std::filesystem::create_directories(std::filesystem::path(resolved).parent_path());
        bool ok = SceneSerializer::Save(resolved, snapshot);
        if (editor) {
            editor->currentMapName = std::filesystem::path(resolved).stem().string();
            editor->setStatusMessage(ok ? std::string("Map saved: ") + editor->currentMapName
                                         : std::string("Map save failed: ") + editor->currentMapName,
                                     ok ? 3.0f : 5.0f);
        }
        return ok;
    }

    bool saveActiveMap() { return saveSceneTo(currentMapPath()); }

    bool loadSceneFrom(const std::string& path) {
        if (!models || !scene) return false;
        auto snapshot = SceneSerializer::Load(path);
        if (!snapshot) {
            if (editor) editor->setStatusMessage("Failed to load map", 5.0f);
            return false;
        }
        scene->setLight(snapshot->light);
        scene->setEnvironment(snapshot->environment);
        models->loadInstances(snapshot->models);
        if (editor) {
            editor->currentMapName = std::filesystem::path(path).stem().string();
            editor->setStatusMessage(std::string("Map loaded: ") + editor->currentMapName, 3.0f);
        }
        return true;
    }

    bool createSceneAt(const std::string& path) {
        if (!models || !scene) return false;
        SceneSnapshot backup;
        backup.light = scene->light();
        backup.environment = scene->environment();
        backup.models = models->serializeInstances();
        std::string previousName = editor ? editor->currentMapName : std::string();

        models->clear();
        DirectionalLightSettings defaultLight;
        EnvironmentSettings defaultEnv;
        scene->setLight(defaultLight);
        scene->setEnvironment(defaultEnv);
        bool ok = saveSceneTo(path);

        models->loadInstances(backup.models);
        scene->setLight(backup.light);
        scene->setEnvironment(backup.environment);
        if (editor) {
            editor->currentMapName = previousName;
            editor->setStatusMessage(ok ? "New map created" : "Failed to create map", ok ? 3.0f : 5.0f);
        }
        return ok;
    }
};

int main()
{
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a GLFW window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Model Viewer", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    // Configure global opengl state
    glEnable(GL_DEPTH_TEST);

    // Build and compile our shader program
    Shader ourShader("../shaders/model_loading.vs", "../shaders/model_loading.fs");

    // Model manager with drag-and-drop support
    ModelManager manager;
    ModelManager::InstallDropHandler(window, &manager);

    SceneState sceneState;
    EditorState editorState;
    const std::string modelsRoot = "../resources/models";
    const std::string mapsRoot = "../resources/maps";

    AppContext appContext{ &manager, &sceneState, &editorState, mapsRoot };

    UiOverlay overlay;
    auto saveCb = [&appContext](const std::string& path) { return appContext.saveSceneTo(path); };
    auto loadCb = [&appContext](const std::string& path) { return appContext.loadSceneFrom(path); };
    auto newCb  = [&appContext](const std::string& path) { return appContext.createSceneAt(path); };
    overlay.init(window,
                modelsRoot,
                mapsRoot,
                &manager,
                &camera,
                &sceneState,
                &editorState,
                saveCb,
                loadCb,
                newCb);
    bool prevToggleE = false;
    bool prevSaveCombo = false;

    // Grid floor
    Shader gridShader("../shaders/grid.vs", "../shaders/grid.fs");
    GridRenderer grid;
    
    // Draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        if (deltaTime > 0.0f) {
            editorState.fps = 1.0f / deltaTime;
        }
        editorState.update(deltaTime);

        // Input (disable camera controls when cursor is not disabled)
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
            processInput(window);
        }

        bool ctrlDown = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
        bool sDown = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
        bool saveCombo = ctrlDown && sDown;
        if (saveCombo && !prevSaveCombo) {
            appContext.saveActiveMap();
        }
        prevSaveCombo = saveCombo;

        // Toggle UI with E
        bool eDown = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;
        if (eDown && !prevToggleE) {
            overlay.toggleVisible();
        }
        prevToggleE = eDown;

        // Render
        glm::vec3 skyColor = sceneState.environment().skyColor;
        glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Begin UI frame
        overlay.beginFrame();

        // Don't forget to enable shader before setting uniforms
        ourShader.use();

        // View/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // Lighting uniforms
        ourShader.setVec3("viewPos", camera.Position);
        sceneState.applyLighting(ourShader);

        // Draw grid
        if (editorState.gridVisible) {
            grid.draw(gridShader, view, projection);
        }

        // Draw placed models
        manager.drawAll(ourShader, editorState.highlightObjects, editorState.highlightColor);

        // Placement preview follows camera until click
        if (manager.hasPreview()) {
            glm::vec3 forward = camera.Front;
            glm::vec3 pos = camera.Position + forward * 3.0f; // 3 units in front
            pos.y = 0.0f; // snap to ground plane
            manager.setPreviewPosition(pos);
            manager.drawPreview(ourShader, true, editorState.highlightColor);

            ImGuiIO& io = ImGui::GetIO();
            bool mouseCaptured = io.WantCaptureMouse;
            bool leftDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
            if (leftDown && !mouseCaptured) {
                manager.confirmPlacement();
            }
        }

        // Draw UI
        overlay.draw();
        overlay.endFrame();

        // GLFW: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}

// Process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}

// GLFW: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // Make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// GLFW: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
        return; // ignore mouse look when UI is open / cursor visible
    }
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// GLFW: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
