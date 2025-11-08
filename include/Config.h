#ifndef CONFIG_H
#define CONFIG_H

// Configuration de la fenêtre
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;
const char* WINDOW_TITLE = "3D Sandbox";

// Chemins des ressources
const char* MODEL_PATH = "resources/models/backpack/backpack.obj";

// Chemins des shaders
namespace ShaderPaths {
    const char* MODEL_VS = "shaders/model_loading.vs";
    const char* MODEL_FS = "shaders/model_loading.fs";
}

// Configuration de la caméra
namespace CameraConfig {
    const float YAW         = -90.0f;
    const float PITCH       =  0.0f;
    const float SPEED       =  2.5f;
    const float SENSITIVITY =  0.1f;
    const float ZOOM        =  45.0f;
    const glm::vec3 POSITION = glm::vec3(0.0f, 0.0f, 3.0f);
    const glm::vec3 UP       = glm::vec3(0.0f, 1.0f, 0.0f);
    const glm::vec3 FRONT    = glm::vec3(0.0f, 0.0f, -1.0f);
}

// Configuration de la lumière
namespace LightConfig {
    const glm::vec3 POSITION  = glm::vec3(1.2f, 1.0f, 2.0f);
    const glm::vec3 AMBIENT   = glm::vec3(0.2f, 0.2f, 0.2f);
    const glm::vec3 DIFFUSE   = glm::vec3(0.5f, 0.5f, 0.5f);
    const glm::vec3 SPECULAR  = glm::vec3(1.0f, 1.0f, 1.0f);
    const float SHININESS     = 32.0f;
}

#endif // CONFIG_H
