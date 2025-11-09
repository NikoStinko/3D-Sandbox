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
    try {
        std::string content;
        std::ifstream fileStream(filePath, std::ios::in);
        
        if (!fileStream.is_open()) {
            std::string errorMsg = "ERREUR: Impossible d'ouvrir le fichier: " + std::string(filePath);
            throw std::runtime_error(errorMsg);
        }
        
        std::string line;
        while (std::getline(fileStream, line)) {
            content += line + "\n";
        }
        
        if (content.empty()) {
            std::cerr << "ATTENTION: Le fichier shader est vide: " << filePath << std::endl;
        }
        
        fileStream.close();
        return content;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception dans loadShaderSource: " << e.what() << std::endl;
        return "";
    }
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
    try {
        // Initialisation de GLFW
        if (!glfwInit()) {
            throw std::runtime_error("Échec de l'initialisation de GLFW");
        }

    // Configuration de GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Création de la fenêtre
        GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, WINDOW_TITLE, NULL, NULL);
        if (!window) {
            throw std::runtime_error("Échec de la création de la fenêtre GLFW");
        }
        glfwMakeContextCurrent(window);
        
        // Configuration du callback de redimensionnement
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        // Initialisation de GLEW
        glewExperimental = GL_TRUE;
        GLenum glewError = glewInit();
        if (glewError != GLEW_OK) {
            std::string error = "Échec de l'initialisation de GLEW: ";
            error += reinterpret_cast<const char*>(glewGetErrorString(glewError));
            throw std::runtime_error(error);
        }
        
        // Vérification de la version d'OpenGL
        if (!GLEW_VERSION_3_3) {
            throw std::runtime_error("OpenGL 3.3 n'est pas disponible sur ce système");
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
        Shader ourShader;
        try {
            std::cout << "Chargement des shaders..." << std::endl;
            ourShader = Shader(ShaderPaths::MODEL_VS, ShaderPaths::MODEL_FS);
            std::cout << "Shaders chargés avec succès" << std::endl;
        } catch (const std::exception& e) {
            std::string error = "Erreur lors du chargement des shaders: ";
            error += e.what();
            throw std::runtime_error(error);
        }

    // Chargement du modèle
    Model ourModel;
    try {
        std::cout << "Chargement du modèle: " << MODEL_PATH << std::endl;
        ourModel = Model(MODEL_PATH);
        std::cout << "Modèle chargé avec succès" << std::endl;
        
        // Afficher des informations sur le modèle
        std::cout << "Nombre de maillages: " << ourModel.meshes.size() << std::endl;
        std::cout << "Nombre de textures chargées: " << ourModel.textures_loaded.size() << std::endl;
        
        // Afficher les chemins des textures chargées
        for (const auto& texture : ourModel.textures_loaded) {
            std::cout << "Texture chargée - Type: " << texture.type 
                      << ", Chemin: " << texture.path << std::endl;
        }
    } catch (const std::exception& e) {
        std::string error = "Erreur lors du chargement du modèle: ";
        error += e.what();
        throw std::runtime_error(error);
    }

    // Configuration de la lumière
    glm::vec3 lightPos(5.0f, 5.0f, 5.0f);  // Position plus éloignée pour une meilleure visibilité
    
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
        ourShader.setFloat("material.shininess", 16.0f);

        // Configuration de la lumière
        ourShader.setVec3("light.position", lightPos);
        ourShader.setVec3("light.ambient", 0.5f, 0.5f, 0.5f);
        ourShader.setVec3("light.diffuse", 1.0f, 1.0f, 1.0f);
        ourShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
        
        // Afficher des informations de débogage (à supprimer plus tard)
        static bool debugShown = false;
        if (!debugShown) {
            std::cout << "\nConfiguration de la lumière :\n";
            std::cout << "- Position: (" << lightPos.x << ", " << lightPos.y << ", " << lightPos.z << ")\n";
            std::cout << "- Vue position: (" << camera.Position.x << ", " << camera.Position.y << ", " << camera.Position.z << ")\n";
            debugShown = true;
        }

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
    catch (const std::exception& e) {
        std::cerr << "\n\n!!! ERREUR CRITIQUE !!!\n";
        std::cerr << "Message d'erreur: " << e.what() << "\n";
        std::cerr << "Type d'exception: " << typeid(e).name() << "\n\n";
        
        // Nettoyage des ressources en cas d'erreur
        glfwTerminate();
        return -1;
    }
}