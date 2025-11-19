#ifndef MODEL_MANAGER_H
#define MODEL_MANAGER_H

#include <memory>
#include <vector>
#include <string>
#include "Model.h"
#include "Log.h"
#include "SceneData.h"
#include <glm/glm.hpp>
#include <optional>

struct GLFWwindow;

class ModelManager {
public:
    ModelManager();
    ~ModelManager();

    void addModel(const std::string &path);
    void addModelInstance(const ModelInstanceData& data);
    void clear();
    void drawAll(Shader &shader, bool highlight = false, const glm::vec3& highlightColor = glm::vec3(1.0f));

    // Placement workflow
    void beginPlacement(const std::string &path);
    bool hasPreview() const { return preview.has_value(); }
    void setPreviewPosition(const glm::vec3 &pos);
    void confirmPlacement();
    void cancelPlacement();
    void drawPreview(Shader &shader, bool highlight = true, const glm::vec3& highlightColor = glm::vec3(1.0f));

    bool hasModels() const { return !models.empty(); }
    std::vector<ModelInstanceData> serializeInstances() const;
    void loadInstances(const std::vector<ModelInstanceData>& data);

    static void InstallDropHandler(GLFWwindow* window, ModelManager* mgr);
    static void DropCallback(GLFWwindow* window, int count, const char** paths);

private:
    struct Entry {
        std::unique_ptr<Model> model;
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;
        std::string path;
    };
    std::vector<Entry> models;
    glm::vec3 nextSpawnOffset = glm::vec3(2.0f, 0.0f, 0.0f);
    glm::vec3 basePosition = glm::vec3(0.0f, 0.0f, 0.0f);
    int count = 0;
    static ComponentLogger logger;

    std::optional<Entry> preview;
};

#endif
