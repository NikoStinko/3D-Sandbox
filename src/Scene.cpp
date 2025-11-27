#include "Scene.h"
#include "EditorState.h"
#include "ModelManager.h"
#include "SceneState.h"
#include "SceneSerializer.h"
#include <filesystem>
#include <iostream>

Scene::Scene(EditorState& e, ModelManager& m, SceneState& s)
    : editor(e), models(m), sceneState(s) {}

bool Scene::createMap(const std::string& name, const std::string& mapsRoot) {
    std::string path = std::filesystem::path(mapsRoot) / (name + ".json");
    editor.currentMapName = name;
    editor.isInMap = true;
    models.clear();
    editor.setStatusMessage(std::string("Map créée: ") + name, 3.0f);
    return true;
}

bool Scene::loadMap(const std::string& name, const std::string& mapsRoot) {
    std::string path = std::filesystem::path(mapsRoot) / (name + ".json");
    auto optSnap = SceneSerializer::Load(path);
    if (!optSnap) {
        editor.setStatusMessage(std::string("Erreur chargement map: ") + name, 3.0f);
        return false;
    }
    const auto& snap = *optSnap;
    sceneState.setLight(snap.light);
    sceneState.setEnvironment(snap.environment);
    models.loadInstances(snap.models);
    editor.currentMapName = name;
    editor.isInMap = true;
    editor.setStatusMessage(std::string("Map chargée: ") + name, 3.0f);
    return true;
}

bool Scene::saveMap(const std::string& name, const std::string& mapsRoot) {
    std::string path = std::filesystem::path(mapsRoot) / (name + ".json");
    SceneSnapshot snap;
    snap.light = sceneState.light();
    snap.environment = sceneState.environment();
    snap.models = models.serializeInstances();
    if (!SceneSerializer::Save(path, snap)) {
        editor.setStatusMessage(std::string("Erreur sauvegarde map: ") + name, 3.0f);
        return false;
    }
    editor.setStatusMessage(std::string("Map sauvegardée: ") + name, 3.0f);
    return true;
}