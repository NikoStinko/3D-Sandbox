#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include "Config.h"
#include "Shader.h"
#include "Model.h"

// Charger les shaders depuis des fichiers
std::string loadShaderSource(const char* filePath) {
    std::string content;
    std::ifstream fileStream(filePath, std::ios::in);
    if (!fileStream.is_open()) {
        std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
        return "";
    }
    std::string line = "";
    while (!fileStream.eof()) {
        std::getline(fileStream, line);
        content.append(line + "\n");
    }
    fileStream.close();
    return content;
}

// Structure pour stocker les informations de texture
struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

// Classe pour gérer les shaders
class Shader {
public:
    unsigned int ID;
    
    Shader(const char* vertexPath, const char* fragmentPath) {
        // 1. Récupérer le code source des shaders
        std::string vertexCode = loadShaderSource(vertexPath);
        std::string fragmentCode = loadShaderSource(fragmentPath);
        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        // 2. Compiler les shaders
        unsigned int vertex, fragment;
        
        // Vertex Shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        
        // Fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        
        // Shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        
        // Supprimer les shaders car ils sont liés au programme
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }
    
    void use() { 
        glUseProgram(ID); 
    }
    
    // Fonctions utilitaires pour les uniforms
    void setBool(const std::string &name, bool value) const {         
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
    }
    void setInt(const std::string &name, int value) const { 
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
    }
    void setFloat(const std::string &name, float value) const { 
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
    }
    void setMat4(const std::string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    void setVec3(const std::string &name, const glm::vec3 &value) const { 
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
    }
    
private:
    void checkCompileErrors(unsigned int shader, std::string type) {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERREUR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        } else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERREUR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};

// Configuration du callback pour le redimensionnement de la fenêtre
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    // Initialisation de GLFW
    if (!glfwInit()) {
        std::cerr << "Échec de l'initialisation de GLFW" << std::endl;
        return -1;
    }

    // Configuration de GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Création de la fenêtre
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (!window) {
        std::cerr << "Échec de la création de la fenêtre GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    // Configuration du callback de redimensionnement
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialisation de GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Échec de l'initialisation de GLEW" << std::endl;
        return -1;
    }
    
    // Configuration d'OpenGL
    glEnable(GL_DEPTH_TEST);
    
    // Configuration du mélange de couleurs pour la transparence
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Configuration du face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    // Affichage des informations de version
    std::cout << "Version d'OpenGL : " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Fabricant : " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Carte graphique : " << glGetString(GL_RENDERER) << std::endl;

    // Configuration d'OpenGL
    glEnable(GL_DEPTH_TEST);

    // Compilation et liaison des shaders
    Shader ourShader("shaders/model_loading.vs", "shaders/model_loading.fs");

    // Configuration de la lumière
    glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
    
    // Boucle de rendu
    while (!glfwWindowShouldClose(window)) {
        // Gestion des entrées
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Rendu
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activation du shader
        ourShader.use();

        // Configuration des matrices de transformation
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f), 
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 
            0.1f, 
            100.0f
        );
        
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        model = glm::rotate(model, (float)glfwGetTime() * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));

        // Envoi des matrices au shader
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        ourShader.setMat4("model", model);

        // Configuration des propriétés des matériaux
        ourShader.setVec3("viewPos", camera.Position);
        ourShader.setFloat("material.shininess", LightConfig::SHININESS);

        // Configuration des lumières
        ourShader.setVec3("light.position", LightConfig::POSITION);
        ourShader.setVec3("light.ambient", LightConfig::AMBIENT);
        ourShader.setVec3("light.diffuse", LightConfig::DIFFUSE);
        ourShader.setVec3("light.specular", LightConfig::SPECULAR);

        // Affichage du modèle chargé
        ourModel.Draw(ourShader);

        // Vérification des erreurs OpenGL
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "Erreur OpenGL pendant le rendu: " << err << std::endl;
        }

        // Échange des tampons et interrogation des événements
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Nettoyage
    glfwTerminate();
    return 0;
}