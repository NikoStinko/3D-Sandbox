#ifndef EDITOR_STATE_H
#define EDITOR_STATE_H

#include <optional>
#include <string>
#include <glm/glm.hpp>

// État des menus
enum class MenuState {
    None,           // Aucun menu affiché
    Radial,         // Menu radial actif
    Pause           // Menu pause actif
};

// Éléments du menu radial
enum class RadialMenuItem {
    None,
    InfoLogs,       // Menu des infos + logs
    ImportModels,   // Menu d'import de modèles
    SceneSettings,  // Paramètres de la scène (lumières, etc.)
    CustomMenu      // Menu personnalisé
};

// Éléments du menu pause
enum class PauseMenuItem {
    None,
    MapList,        // Liste des maps
    RadialMenu,     // Menu radial
    Quit,           // Quitter
    Resume          // Reprendre la simulation
};

struct ObjectSelection {
    size_t modelIndex = 0;
    std::string name;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    float opacity = 1.0f;
};

struct EditorState {
    // États des menus
    MenuState menuState = MenuState::None;
    RadialMenuItem activeRadialItem = RadialMenuItem::None;
    PauseMenuItem activePauseItem = PauseMenuItem::None;
    bool isInMap = false;  // Si on est dans une map
    
    // Paramètres d'affichage
    bool gridVisible = true;
    bool highlightObjects = false;
    glm::vec3 highlightColor = glm::vec3(1.0f, 0.6f, 0.2f);
    
    // Messages et états
    std::string statusMessage;
    float statusMessageTime = 0.0f;
    std::string currentMapName = "default";
    float fps = 0.0f;

    // Sélection d'objet
    std::optional<ObjectSelection> selectedObject;
    bool showObjectProperties = false;

    void setStatusMessage(const std::string& msg, float duration = 3.0f) {
        statusMessage = msg;
        statusMessageTime = duration;
    }

    void update(float deltaTime) {
        if (statusMessageTime > 0.0f) {
            statusMessageTime -= deltaTime;
            if (statusMessageTime <= 0.0f) {
                statusMessageTime = 0.0f;
                statusMessage.clear();
            }
        }
    }
    
    void selectObject(size_t index, const std::string& name, const glm::vec3& pos, 
                     const glm::vec3& rot, const glm::vec3& scl, float opac = 1.0f) {
        selectedObject = {index, name, pos, rot, scl, opac};
        showObjectProperties = true;
    }
    
    void clearSelection() {
        selectedObject = std::nullopt;
        showObjectProperties = false;
    }
};

#endif // EDITOR_STATE_H
