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
        models.emplace_back(std::make_unique<Model>(path));
    } catch (const std::exception &e) {
        logger.error(std::string("Echec ajout modèle: ") + e.what());
    }
}

void ModelManager::drawAll(Shader &shader)
{
    for (auto &m : models) {
        m->Draw(shader);
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
