#ifndef EDITOR_STATE_H
#define EDITOR_STATE_H

#include <string>
#include <glm/glm.hpp>

struct EditorState {
    bool gridVisible = true;
    bool highlightObjects = false;
    glm::vec3 highlightColor = glm::vec3(1.0f, 0.6f, 0.2f);
    std::string statusMessage;
    float statusMessageTime = 0.0f;
    std::string currentMapName = "default";
    float fps = 0.0f;

    void setStatusMessage(const std::string& msg, float duration = 3.0f) {
        statusMessage = msg;
        statusMessageTime = duration;
    }

    void update(float deltaTime) {
        if (statusMessageTime > 0.0f) {
            statusMessageTime -= deltaTime;
            if (statusMessageTime <= 0.0f) {
                statusMessageTime = 0.0f;
                statusMessage.clear();
            }
        }
    }
};

#endif // EDITOR_STATE_H
