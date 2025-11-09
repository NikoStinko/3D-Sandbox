#ifndef MODEL_MANAGER_H
#define MODEL_MANAGER_H

#include <memory>
#include <vector>
#include <string>
#include "Model.h"
#include "Log.h"

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
    std::vector<std::unique_ptr<Model>> models;
    static ComponentLogger logger;
};

#endif
