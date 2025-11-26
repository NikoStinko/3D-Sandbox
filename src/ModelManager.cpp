#include "ModelManager.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <sstream>

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
        e.path = data.path;
        
        // Calculer l'échelle automatique si nécessaire
        if (data.autoScale) {
            // Obtenir la taille du modèle
            glm::vec3 modelSize = e.model->getModelSize();
            
            // Éviter la division par zéro
            if (modelSize.x > 0.0f && modelSize.y > 0.0f && modelSize.z > 0.0f) {
                // Calculer l'échelle pour normaliser la plus grande dimension à 1.0
                float maxDim = std::max({modelSize.x, modelSize.y, modelSize.z});
                float scaleFactor = 1.0f / maxDim;
                
                // Appliquer l'échelle de base du modèle
                e.scale = glm::vec3(scaleFactor) * data.scale;
                
                std::stringstream ss;
                ss << "Mise à l'échelle automatique du modèle: "
                   << "taille=" << modelSize.x << "x" << modelSize.y << "x" << modelSize.z
                   << ", facteur d'échelle=" << scaleFactor;
                logger.info(ss.str());
            } else {
                e.scale = glm::vec3(1.0f);
                logger.info("Impossible de calculer l'échelle automatique, utilisation de l'échelle par défaut");
            }
        } else {
            e.scale = data.scale;
        }
        
        // Éviter une échelle nulle
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

bool ModelManager::raycast(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, size_t& outIndex) {
    float closestT = std::numeric_limits<float>::max();
    bool hit = false;
    
    for (size_t i = 0; i < models.size(); ++i) {
        const auto& entry = models[i];
        
        // Simple AABB test pour l'instant (peut être amélioré avec un test plus précis)
        glm::vec3 minBounds = entry.position - entry.scale * 0.5f;
        glm::vec3 maxBounds = entry.position + entry.scale * 0.5f;
        
        float t1 = (minBounds.x - rayOrigin.x) / rayDirection.x;
        float t2 = (maxBounds.x - rayOrigin.x) / rayDirection.x;
        float t3 = (minBounds.y - rayOrigin.y) / rayDirection.y;
        float t4 = (maxBounds.y - rayOrigin.y) / rayDirection.y;
        float t5 = (minBounds.z - rayOrigin.z) / rayDirection.z;
        float t6 = (maxBounds.z - rayOrigin.z) / rayDirection.z;
        
        float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
        float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));
        
        if (tmax >= 0 && tmin <= tmax) {
            if (tmin < closestT) {
                closestT = tmin;
                outIndex = i;
                hit = true;
            }
        }
    }
    
    return hit;
}

void ModelManager::updateModel(size_t index, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale, float opacity) {
    if (index < models.size()) {
        models[index].position = position;
        models[index].rotation = rotation;
        models[index].scale = scale;
        // L'opacité devra être gérée dans le shader
    }
}

const ModelManager::Entry* ModelManager::getModel(size_t index) const {
    if (index < models.size()) {
        return &models[index];
    }
    return nullptr;
}
