#include "Renderer.h"
#include "Shader.h"
#include "ModelManager.h"
#include "Grid.h"
#include "Camera.h"
#include "EditorState.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void Renderer::render(Shader& shader, ModelManager& modelManager, Grid& grid, const Camera& camera, float deltaTime, const EditorState& editorState) {
    shader.use();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)1600 / (float)900, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    shader.setVec3("viewPos", camera.Position);
    shader.setVec3("lightPos", glm::vec3(1.2f, 1.0f, 2.0f));
    shader.setFloat("time", deltaTime);

    modelManager.drawAll(shader, editorState.highlightObjects, editorState.highlightColor);

    if (editorState.gridVisible) {
        glm::mat4 gridModel = glm::mat4(1.0f);
        shader.setMat4("model", gridModel);
        grid.draw(shader, view, projection);
    }
}