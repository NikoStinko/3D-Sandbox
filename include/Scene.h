#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <functional>
#include "EditorState.h"
#include "ModelManager.h"
#include "SceneState.h"

class EditorState;
class ModelManager;
class SceneState;

class Scene {
public:
    Scene(EditorState& editor, ModelManager& models, SceneState& sceneState);

    bool createMap(const std::string& name, const std::string& mapsRoot);
    bool loadMap(const std::string& name, const std::string& mapsRoot);
    bool saveMap(const std::string& name, const std::string& mapsRoot);
    bool isInMap() const { return editor.isInMap; }

private:
    EditorState& editor;
    ModelManager& models;
    SceneState& sceneState;
};

#endif // SCENE_H