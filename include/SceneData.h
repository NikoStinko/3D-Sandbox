#ifndef SCENE_DATA_H
#define SCENE_DATA_H

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "SceneState.h"

struct ModelInstanceData {
    std::string path;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    bool autoScale = false; // Désactive la mise à l'échelle automatique par défaut
};

struct SceneSnapshot {
    DirectionalLightSettings light;
    EnvironmentSettings environment;
    std::vector<ModelInstanceData> models;
};

#endif // SCENE_DATA_H
