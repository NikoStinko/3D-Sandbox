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
#include "MenuRenderer.h"
#include "SandBoxUI.h"
#include <imgui.h>
#include <iostream>
#include <filesystem>
#include <cmath>

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, EditorState& editorState, ModelManager& modelManager, Camera& camera, UiOverlay& overlay, bool& overlayOpenedForPause);
unsigned int loadTexture(const char *path);

// Window dimensions
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

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
    bool overlayOpenedForPause = false;

    SandBoxUI sandboxUI(&manager, &editorState, &camera);

    // Grid floor
    Shader gridShader("../shaders/grid.vs", "../shaders/grid.fs");
    Grid grid;
    
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
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED || overlay.isVisible() || editorState.menuState != MenuState::None) {
            processInput(window, editorState, manager, camera, overlay, overlayOpenedForPause);
        }
        
        // Mettre à jour l'état de l'éditeur
        editorState.update(deltaTime);
        
        // Mettre à jour les FPS
        static float fpsTimer = 0.0f;
        static int frameCount = 0;
        fpsTimer += deltaTime;
        frameCount++;
        if (fpsTimer >= 0.5f) {
            editorState.fps = static_cast<float>(frameCount) / fpsTimer;
            frameCount = 0;
            fpsTimer = 0.0f;
        }

        bool ctrlDown = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
        bool sDown = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
        bool saveCombo = ctrlDown && sDown;
        if (saveCombo && !prevSaveCombo) {
            appContext.saveActiveMap();
        }
        prevSaveCombo = saveCombo;

        // Toggle UI with E (désactivé pour permettre le menu radial)
        // bool eDown = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;
        // if (eDown && !prevToggleE) {
        //     overlay.toggleVisible();
        // }
        // prevToggleE = eDown;

        // Render
        glm::vec3 skyColor = sceneState.environment().skyColor;
        glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Begin UI frame
        overlay.beginFrame();

        // UI métier centralisée (picking triangulation, menu contextuel)
        sandboxUI.draw(window);

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
            // Ne gérer le placement que si aucun menu n'est ouvert
            if (leftDown && !mouseCaptured && editorState.menuState == MenuState::None) {
                manager.confirmPlacement();
            }
        }

        // Draw UI
        overlay.draw();
        
        // Dessiner les menus si nécessaire (après overlay.draw)
        if (editorState.menuState == MenuState::Radial) {
            ImGuiIO& io = ImGui::GetIO();
            glm::vec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
            float radius = 150.0f;
            MenuRenderer::drawRadialMenu(editorState, center, radius);
        }
        
        // Gérer les clics dans les menus (après avoir dessiné)
        static bool leftMouseWasPressed = false;
        bool leftMouseIsPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        
        if (leftMouseIsPressed && !leftMouseWasPressed && editorState.menuState != MenuState::None) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            glm::vec2 clickPos(static_cast<float>(xpos), static_cast<float>(ypos));
            
            if (editorState.menuState == MenuState::Radial) {
                ImGuiIO& io = ImGui::GetIO();
                glm::vec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
                float radius = 150.0f;
                
                RadialMenuItem clickedItem = MenuRenderer::handleRadialMenuClick(editorState, clickPos, center, radius);
                if (clickedItem != RadialMenuItem::None) {
                    switch (clickedItem) {
                        case RadialMenuItem::InfoLogs:
                            // Afficher les infos/logs (panneau de scène)
                            editorState.menuState = MenuState::None;
                            editorState.activeRadialItem = RadialMenuItem::None;
                            overlay.showOnlyScenePanel();
                            break;
                        case RadialMenuItem::ImportModels:
                            // Afficher le navigateur de modèles (via SandBoxUI)
                            editorState.menuState = MenuState::None;
                            editorState.activeRadialItem = RadialMenuItem::None;
                            sandboxUI.openModelBrowser(overlay);
                            break;
                        case RadialMenuItem::SceneSettings:
                            // Afficher le panneau de paramètres de scène (via SandBoxUI)
                            editorState.menuState = MenuState::None;
                            editorState.activeRadialItem = RadialMenuItem::None;
                            sandboxUI.openScenePanel(overlay);
                            break;
                        case RadialMenuItem::CustomMenu:
                            // Afficher le panneau personnalisé
                            editorState.menuState = MenuState::None;
                            editorState.activeRadialItem = RadialMenuItem::None;
                            overlay.showOnlyCustomButtons();
                            break;
                        default:
                            break;
                    }
                }
            } else if (editorState.menuState == MenuState::Pause && !overlay.isVisible()) {
                PauseMenuItem clickedItem = MenuRenderer::handlePauseMenuClick(editorState, clickPos);
                if (clickedItem != PauseMenuItem::None) {
                    switch (clickedItem) {
                        case PauseMenuItem::MapList:
                            // Afficher uniquement le panel des maps (via SandBoxUI)
                            editorState.menuState = MenuState::None;
                            editorState.activePauseItem = PauseMenuItem::None;
                            sandboxUI.openMapPanel(overlay);
                            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                            break;
                        case PauseMenuItem::RadialMenu:
                            editorState.menuState = MenuState::Radial;
                            editorState.activePauseItem = PauseMenuItem::None;
                            // Si l'overlay a été ouvert à cause de l'ESC, le fermer pour le menu radial
                            if (overlayOpenedForPause) { overlay.toggleVisible(); overlayOpenedForPause = false; }
                            break;
                        case PauseMenuItem::Quit:
                            glfwSetWindowShouldClose(window, true);
                            break;
                        case PauseMenuItem::Resume:
                            editorState.menuState = MenuState::None;
                            editorState.activePauseItem = PauseMenuItem::None;
                            // Fermer l'overlay si ouvert pour pause
                            if (overlayOpenedForPause) { overlay.toggleVisible(); overlayOpenedForPause = false; }
                            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        leftMouseWasPressed = leftMouseIsPressed;
        
        overlay.endFrame();

        // GLFW: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return -1;
}

// Fonction de rappel pour le redimensionnement de la fenêtre
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Fonction de rappel pour le mouvement de la souris
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    static float lastX = SCR_WIDTH / 2.0f;
    static float lastY = SCR_HEIGHT / 2.0f;
    static bool firstMouse = true;

    if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
        return; // Ignorer le mouvement de la souris quand l'UI est ouverte
    }

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Inversé car les coordonnées Y vont de bas en haut

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// Fonction de rappel pour la molette de défilement
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// Fonction de traitement des entrées clavier
void processInput(GLFWwindow* window, EditorState& editorState, ModelManager& modelManager, Camera& camera, UiOverlay& overlay, bool& overlayOpenedForPause) {
    static bool escapeWasPressed = false;
    static bool tabWasPressed = false;
    static float lastFrame = 0.0f;
    
    // Calcul du deltaTime
    float currentFrame = static_cast<float>(glfwGetTime());
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Gestion des clics dans les menus
    static bool leftMouseWasPressed = false;
    bool leftMouseIsPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    
    leftMouseWasPressed = leftMouseIsPressed;

    // Gestion de la touche Échap (menu pause et fermeture des panneaux)
    bool escapeIsPressed = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
    if (escapeIsPressed && !escapeWasPressed) {
        if (editorState.menuState == MenuState::Pause) {
            // Quitter le menu pause
            editorState.menuState = MenuState::None;
            editorState.activePauseItem = PauseMenuItem::None;
            // Si l'overlay a été ouvert pour la pause, le fermer
            if (overlayOpenedForPause && overlay.isVisible()) { overlay.toggleVisible(); overlayOpenedForPause = false; }
        } else if (editorState.menuState == MenuState::Radial) {
            // Retour au menu pause
            editorState.menuState = MenuState::Pause;
            editorState.activePauseItem = PauseMenuItem::None;
            // S'assurer que l'overlay est visible pour afficher le menu pause (style overlay)
            if (!overlay.isVisible()) { overlay.toggleVisible(); overlayOpenedForPause = true; }
        } else if (overlay.isVisible()) {
            // Fermer toutes les fenêtres UI si aucune pause n'est active
            overlay.hideAllPanels();
            overlay.toggleVisible();
            overlayOpenedForPause = false;
        } else {
            // Ouvrir le menu pause avec le style overlay
            editorState.menuState = MenuState::Pause;
            editorState.activePauseItem = PauseMenuItem::Resume; // Sélection par défaut
            // Masquer tous les panneaux pour n'afficher que le menu pause overlay
            overlay.hideAllPanels();
            if (!overlay.isVisible()) { overlay.toggleVisible(); overlayOpenedForPause = true; }
        }
    }
    
    // Gérer l'état du curseur en fonction de l'état du menu et de l'overlay
    if (editorState.menuState != MenuState::None || overlay.isVisible()) {
        if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_NORMAL) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    } else {
        if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
    escapeWasPressed = escapeIsPressed;

    // Gestion de la touche Tab (menu radial)
    bool tabIsPressed = glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS;
    if (tabIsPressed && !tabWasPressed && editorState.menuState != MenuState::Pause) {
        // Basculer le menu radial (pas en pause)
        if (editorState.menuState == MenuState::Radial) {
            editorState.menuState = MenuState::None;
            editorState.activeRadialItem = RadialMenuItem::None;
            // Capturer le curseur à nouveau
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            editorState.menuState = MenuState::Radial;
            editorState.activeRadialItem = RadialMenuItem::None;
            // Si l'overlay a été ouvert pour pause, le fermer pour éviter superposition
            if (overlayOpenedForPause && overlay.isVisible()) { overlay.toggleVisible(); overlayOpenedForPause = false; }
            // Libérer le curseur pour permettre la sélection avec la souris
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    tabWasPressed = tabIsPressed;

    // Si un menu est actif, on ne traite pas les déplacements de la caméra
    if (editorState.menuState != MenuState::None) {
        return;
    }

    // Déplacements de la caméra
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