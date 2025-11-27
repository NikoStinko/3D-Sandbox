#ifndef SANDBOX_UI_H
#define SANDBOX_UI_H

#include <glm/glm.hpp>
#include <cstddef>
#include <string>

struct GLFWwindow;
class UiOverlay;
class ModelManager;
class EditorState;
class Camera;

class SandBoxUI {
public:
    SandBoxUI(ModelManager* models, EditorState* editor, Camera* camera);

    // Dessine l’UI métier (menu contextuel + fenêtres Resize/Rotate)
    void draw(GLFWwindow* window);

    // Helpers d’ouverture avec règles d’accès
    void openModelBrowser(UiOverlay& overlay);
    void openScenePanel(UiOverlay& overlay);
    void openMapPanel(UiOverlay& overlay);

private:
    ModelManager* models;
    EditorState* editor;
    Camera* camera;

    // Picking précis
    bool pickUnderCursor(GLFWwindow* window, size_t& outIndex, glm::vec3& outHitPoint);
    static bool intersectRayTriangle(const glm::vec3& orig, const glm::vec3& dir,
                                     const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                                     float& outT, float& outU, float& outV);
    bool screenRay(GLFWwindow* window, glm::vec3& outOrigin, glm::vec3& outDir) const;

    // État menu contextuel
    bool showResizePopup = false;
    bool showRotatePopup = false;
    size_t contextIndex = 0;
    glm::vec3 tmpScale = glm::vec3(1.0f);
    glm::vec3 tmpRotation = glm::vec3(0.0f);
};

#endif // SANDBOX_UI_H