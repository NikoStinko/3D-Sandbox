#ifndef MODEL_MANAGER_H
#define MODEL_MANAGER_H

#include <memory>
#include <vector>
#include <string>
#include "Model.h"
#include "Log.h"
#include <glm/glm.hpp>

struct GLFWwindow;

class ModelManager {
public:
    ModelManager();
    ~ModelManager();

    void addModel(const std::string &path);
    void drawAll(Shader &shader);

    static void InstallDropHandler(GLFWwindow* window, ModelManager* mgr);
    static void DropCallback(GLFWwindow* window, int count, const char** paths);

private:
    struct Entry {
        std::unique_ptr<Model> model;
        glm::vec3 position;
    };
    std::vector<Entry> models;
    glm::vec3 nextSpawnOffset = glm::vec3(2.0f, 0.0f, 0.0f);
    glm::vec3 basePosition = glm::vec3(0.0f, 0.0f, 0.0f);
    int count = 0;
    static ComponentLogger logger;
};

#endif
