#include "SceneState.h"
#include "Shader.h"

#include <glm/gtc/type_ptr.hpp>

void SceneState::applyLighting(Shader& shader) const
{
    glm::vec3 dir = glm::normalize(lightSettings.direction);
    glm::vec3 ambient = lightSettings.ambient * lightSettings.intensity;
    glm::vec3 diffuse = lightSettings.diffuse * lightSettings.intensity;
    glm::vec3 specular = lightSettings.specular * lightSettings.intensity;

    shader.setVec3("dirLight.direction", dir);
    shader.setVec3("dirLight.ambient", ambient);
    shader.setVec3("dirLight.diffuse", diffuse);
    shader.setVec3("dirLight.specular", specular);
    shader.setFloat("environmentAmbientBoost", environmentSettings.ambientBoost);
}

void SceneState::setLight(const DirectionalLightSettings& settings)
{
    lightSettings = settings;
}

void SceneState::setEnvironment(const EnvironmentSettings& settings)
{
    environmentSettings = settings;
}
