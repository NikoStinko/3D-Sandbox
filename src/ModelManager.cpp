#include "ModelManager.h"
#include <GLFW/glfw3.h>
#include <iostream>

ComponentLogger ModelManager::logger("Model");

ModelManager::ModelManager() {}
ModelManager::~ModelManager() {}

void ModelManager::addModel(const std::string &path)
{
    try {
        logger.info(std::string("Ajout du modèle: ") + path);
        Entry e;
        e.model = std::make_unique<Model>(path);
        // simple spacing logic: offset each new model along X
        e.position = basePosition + float(count) * nextSpawnOffset;
        models.emplace_back(std::move(e));
        count++;
    } catch (const std::exception &e) {
        logger.error(std::string("Echec ajout modèle: ") + e.what());
    }
}

void ModelManager::drawAll(Shader &shader)
{
    for (auto &e : models) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, e.position);
        shader.setMat4("model", model);
        e.model->Draw(shader);
    }
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
