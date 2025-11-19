#ifndef SCENE_STATE_H
#define SCENE_STATE_H

#include <glm/glm.hpp>

class Shader;

struct DirectionalLightSettings {
    glm::vec3 direction = glm::vec3(-0.3f, -1.0f, -0.2f);
    glm::vec3 ambient = glm::vec3(0.2f);
    glm::vec3 diffuse = glm::vec3(0.8f);
    glm::vec3 specular = glm::vec3(1.0f);
    float intensity = 1.0f;
};

struct EnvironmentSettings {
    glm::vec3 skyColor = glm::vec3(0.05f, 0.06f, 0.10f);
    float ambientBoost = 1.0f;
};

class SceneState {
public:
    DirectionalLightSettings& light() { return lightSettings; }
    const DirectionalLightSettings& light() const { return lightSettings; }

    EnvironmentSettings& environment() { return environmentSettings; }
    const EnvironmentSettings& environment() const { return environmentSettings; }

    void applyLighting(Shader& shader) const;
    void setLight(const DirectionalLightSettings& settings);
    void setEnvironment(const EnvironmentSettings& settings);

private:
    DirectionalLightSettings lightSettings;
    EnvironmentSettings environmentSettings;
};

#endif // SCENE_STATE_H
