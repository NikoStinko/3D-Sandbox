#include "ModelManager.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

ComponentLogger ModelManager::logger("Model");

ModelManager::ModelManager() {}
ModelManager::~ModelManager() {}

void ModelManager::addModel(const std::string &path)
{
    ModelInstanceData data;
    data.path = path;
    data.position = basePosition + float(count) * nextSpawnOffset;
    data.rotation = glm::vec3(0.0f);
    data.scale = glm::vec3(1.0f);
    addModelInstance(data);
}

void ModelManager::addModelInstance(const ModelInstanceData& data)
{
    if (data.path.empty()) {
        logger.error("Chemin vide pour addModelInstance");
        return;
    }
    try {
        logger.info(std::string("Ajout du modèle: ") + data.path);
        Entry e;
        e.model = std::make_unique<Model>(data.path);
        e.position = data.position;
        e.rotation = data.rotation;
        e.scale = data.scale;
        e.path = data.path;
        if (e.scale == glm::vec3(0.0f)) {
            e.scale = glm::vec3(1.0f);
        }
        models.emplace_back(std::move(e));
        count++;
    } catch (const std::exception &ex) {
        logger.error(std::string("Echec ajout modèle: ") + ex.what());
    }
}

void ModelManager::clear()
{
    models.clear();
    preview.reset();
    count = 0;
}

void ModelManager::drawAll(Shader &shader, bool highlight, const glm::vec3& highlightColor)
{
    for (auto &e : models) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, e.position);
        model = glm::rotate(model, glm::radians(e.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(e.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(e.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, e.scale == glm::vec3(0.0f) ? glm::vec3(1.0f) : e.scale);
        shader.setBool("highlightActive", highlight);
        shader.setVec3("highlightColor", highlightColor);
        shader.setMat4("model", model);
        e.model->Draw(shader);
    }
}

void ModelManager::beginPlacement(const std::string &path)
{
    try {
        logger.info(std::string("Begin placement: ") + path);
        Entry e;
        e.model = std::make_unique<Model>(path);
        e.position = glm::vec3(0.0f);
        e.rotation = glm::vec3(0.0f);
        e.scale = glm::vec3(1.0f);
        e.path = path;
        preview = std::move(e);
    } catch (const std::exception &ex) {
        logger.error(std::string("Echec beginPlacement: ") + ex.what());
        preview.reset();
    }
}

void ModelManager::setPreviewPosition(const glm::vec3 &pos)
{
    if (preview) preview->position = pos;
}

void ModelManager::confirmPlacement()
{
    if (!preview) return;
    logger.info("Confirm placement");
    ModelInstanceData data;
    data.path = preview->path;
    data.position = preview->position;
    data.rotation = preview->rotation;
    data.scale = preview->scale;
    addModelInstance(data);
    preview.reset();
}

void ModelManager::cancelPlacement()
{
    logger.info("Cancel placement");
    preview.reset();
}

void ModelManager::drawPreview(Shader &shader, bool highlight, const glm::vec3& highlightColor)
{
    if (!preview) return;
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, preview->position);
    model = glm::rotate(model, glm::radians(preview->rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(preview->rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(preview->rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, preview->scale == glm::vec3(0.0f) ? glm::vec3(1.0f) : preview->scale);
    shader.setBool("highlightActive", highlight);
    shader.setVec3("highlightColor", highlightColor);
    shader.setMat4("model", model);
    preview->model->Draw(shader);
}

std::vector<ModelInstanceData> ModelManager::serializeInstances() const
{
    std::vector<ModelInstanceData> result;
    result.reserve(models.size());
    for (const auto& entry : models) {
        ModelInstanceData data;
        data.path = entry.path;
        data.position = entry.position;
        data.rotation = entry.rotation;
        data.scale = entry.scale;
        result.push_back(std::move(data));
    }
    return result;
}

void ModelManager::InstallDropHandler(GLFWwindow* window, ModelManager* mgr)
{
    glfwSetWindowUserPointer(window, mgr);
    glfwSetDropCallback(window, &ModelManager::DropCallback);
}

void ModelManager::DropCallback(GLFWwindow* window, int count, const char** paths)
{
    auto *mgr = reinterpret_cast<ModelManager*>(glfwGetWindowUserPointer(window));
    if (!mgr) return;
    for (int i = 0; i < count; ++i) {
        if (paths[i]) mgr->addModel(std::string(paths[i]));
    }
}

void ModelManager::loadInstances(const std::vector<ModelInstanceData>& data)
{
    clear();
    for (const auto& entry : data) {
        addModelInstance(entry);
    }
}
