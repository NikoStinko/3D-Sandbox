#include "SceneSerializer.h"

#include <fstream>
#include <filesystem>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace {
json glmVec3ToJson(const glm::vec3& v) {
    return json{v.x, v.y, v.z};
}

glm::vec3 jsonToVec3(const json& j, const glm::vec3& fallback = glm::vec3(0.0f)) {
    if (!j.is_array() || j.size() != 3) return fallback;
    return glm::vec3(j[0].get<float>(), j[1].get<float>(), j[2].get<float>());
}
}

bool SceneSerializer::Save(const std::string& path, const SceneSnapshot& snapshot)
{
    json j;
    j["light"] = {
        {"direction", glmVec3ToJson(snapshot.light.direction)},
        {"ambient", glmVec3ToJson(snapshot.light.ambient)},
        {"diffuse", glmVec3ToJson(snapshot.light.diffuse)},
        {"specular", glmVec3ToJson(snapshot.light.specular)},
        {"intensity", snapshot.light.intensity}
    };

    j["environment"] = {
        {"skyColor", glmVec3ToJson(snapshot.environment.skyColor)},
        {"ambientBoost", snapshot.environment.ambientBoost}
    };

    json modelsJson = json::array();
    for (const auto& model : snapshot.models) {
        modelsJson.push_back({
            {"path", model.path},
            {"position", glmVec3ToJson(model.position)},
            {"rotation", glmVec3ToJson(model.rotation)},
            {"scale", glmVec3ToJson(model.scale)}
        });
    }
    j["models"] = modelsJson;

    std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    std::ofstream out(path);
    if (!out) return false;
    out << j.dump(4);
    return true;
}

std::optional<SceneSnapshot> SceneSerializer::Load(const std::string& path)
{
    std::ifstream in(path);
    if (!in) return std::nullopt;
    json j;
    in >> j;

    SceneSnapshot snapshot;
    if (auto lightIt = j.find("light"); lightIt != j.end()) {
        snapshot.light.direction = jsonToVec3((*lightIt)["direction"], snapshot.light.direction);
        snapshot.light.ambient = jsonToVec3((*lightIt)["ambient"], snapshot.light.ambient);
        snapshot.light.diffuse = jsonToVec3((*lightIt)["diffuse"], snapshot.light.diffuse);
        snapshot.light.specular = jsonToVec3((*lightIt)["specular"], snapshot.light.specular);
        snapshot.light.intensity = (*lightIt).value("intensity", snapshot.light.intensity);
    }

    if (auto envIt = j.find("environment"); envIt != j.end()) {
        snapshot.environment.skyColor = jsonToVec3((*envIt)["skyColor"], snapshot.environment.skyColor);
        snapshot.environment.ambientBoost = (*envIt).value("ambientBoost", snapshot.environment.ambientBoost);
    }

    if (auto modelsIt = j.find("models"); modelsIt != j.end() && modelsIt->is_array()) {
        for (const auto& elem : *modelsIt) {
            ModelInstanceData data;
            data.path = elem.value("path", "");
            data.position = jsonToVec3(elem["position"], data.position);
            data.rotation = jsonToVec3(elem["rotation"], data.rotation);
            data.scale = jsonToVec3(elem["scale"], data.scale);
            if (!data.path.empty()) snapshot.models.push_back(std::move(data));
        }
    }

    return snapshot;
}
