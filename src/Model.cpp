#include "Model.h"
#include "Texture.h"
#include "Log.h"

#include <iostream>
#include <vector>
#include <map>
#include <stdexcept>
#include <algorithm>
#include <dirent.h>  // Pour opendir, readdir, closedir
#include <unistd.h>  // Pour getcwd
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <mutex>

// Activation du logging Assimp de base
#define ASSIMP_DO_NOT_USE_GOOGLE_LOGGING

// Mutex pour la synchronisation du chargement des textures
static std::mutex textureMutex;

static ComponentLogger modelLogger("Model");

Model::Model(std::string const &path, bool gamma) : gammaCorrection(gamma)
{
    loadModel(path);
}

void Model::Draw(Shader &shader)
{
    // Activer le shader
    shader.use();
    
    // Dessiner tous les maillages
    for(unsigned int i = 0; i < meshes.size(); i++) {
        meshes[i].Draw(shader);
    }
}

void Model::loadModel(std::string const &path)
{
    modelLogger.info(std::string("Chargement du modele: ") + path);
    
    // Vérifier si le fichier existe
    std::ifstream file(path);
    if (!file.good()) {
        modelLogger.error(std::string("Le fichier du modele n'existe pas: ") + path);
        // Afficher le répertoire de travail actuel
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            modelLogger.debug(std::string("Repertoire de travail actuel: ") + cwd);
        }
        return;
    }
    file.close();
    
    // read file via ASSIMP
    Assimp::Importer importer;
    
    // Logging simple pour Assimp
    modelLogger.debug(std::string("=== CHARGEMENT DU MODELE ===\nChemin du modele: ") + path);
    
    // Vérifier si le fichier existe et est accessible
    std::ifstream fileCheck(path);
    if (!fileCheck.is_open()) {
        modelLogger.error(std::string("Impossible d'ouvrir le fichier du modele: ") + path);
        return;
    }
    fileCheck.close();
    
    // Charger la scène avec des flags supplémentaires pour le débogage
    unsigned int flags = 
        aiProcess_Triangulate | 
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace |
        aiProcess_GenNormals |
        aiProcess_ValidateDataStructure |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ImproveCacheLocality;
        
    modelLogger.debug("Flags de chargement Assimp initialisés");
    
    // Forcer le chargement du fichier .mtl
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_MATERIALS, true);
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_TEXTURES, true);
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_ANIMATIONS, true);
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_LIGHTS, true);
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_CAMERAS, true);
    
    const aiScene* scene = importer.ReadFile(path, flags);
        
    // Afficher des informations sur la scène chargée
    if (scene) {
        modelLogger.debug("Scene chargee: maillages=" + std::to_string(scene->mNumMeshes) + ", materiaux=" + std::to_string(scene->mNumMaterials) + ", textures=" + std::to_string(scene->mTextures ? scene->mNumTextures : 0));
        
        // Afficher les informations sur les matériaux
        for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
            aiMaterial* mat = scene->mMaterials[i];
            aiString name;
            mat->Get(AI_MATKEY_NAME, name);
            modelLogger.debug(std::string("Materiau ") + std::to_string(i) + ": " + name.C_Str());
            
            // Compter les textures de chaque type
            modelLogger.debug("  Textures: Diffuse=" + std::to_string(mat->GetTextureCount(aiTextureType_DIFFUSE)) + ", Specular=" + std::to_string(mat->GetTextureCount(aiTextureType_SPECULAR)) + ", Normal=" + std::to_string(mat->GetTextureCount(aiTextureType_NORMALS)) + ", Height=" + std::to_string(mat->GetTextureCount(aiTextureType_HEIGHT)));
            
            // Afficher les propriétés du matériau
            aiColor3D diffuse, specular, ambient;
            float shininess;
            
            if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS) {
                modelLogger.debug("  - Couleur diffuse renseignee");
            }
            
            if (mat->Get(AI_MATKEY_COLOR_SPECULAR, specular) == AI_SUCCESS) {
                modelLogger.debug("  - Couleur speculaire renseignee");
            }
            
            if (mat->Get(AI_MATKEY_COLOR_AMBIENT, ambient) == AI_SUCCESS) {
                modelLogger.debug("  - Couleur ambiante renseignee");
            }
            
            if (mat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
                modelLogger.debug("  - Brillance renseignee");
            }
        }
        
        // Afficher les informations sur les textures intégrées
        if (scene->mTextures) {
            modelLogger.debug("=== TEXTURES INTEGREES ===");
            for (unsigned int i = 0; i < scene->mNumTextures; i++) {
                aiTexture* tex = scene->mTextures[i];
                modelLogger.debug(std::string("Texture integree: ") + tex->mFilename.C_Str());
            }
        }
    }
        
    // Vérifier les erreurs
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        modelLogger.error(std::string("ASSIMP: ") + importer.GetErrorString());
        return;
    }
    
    // Récupérer le chemin du répertoire du modèle
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        directory = path.substr(0, lastSlash);
    } else {
        // Si pas de slash, le répertoire est le répertoire courant
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            directory = cwd;
        } else {
            directory = ".";
        }
    }
    
    modelLogger.debug(std::string("Repertoire du modele: ") + directory);
    
    modelLogger.debug(std::string("Chemin complet: ") + path);
    
    // Vérifier si le répertoire existe
    DIR* dir = opendir(directory.c_str());
    if (dir) {
        modelLogger.debug("Le repertoire existe");
        closedir(dir);
        
        // Lister les fichiers dans le répertoire
        modelLogger.debug("Listing du repertoire...\n");
        DIR *dirp = opendir(directory.c_str());
        struct dirent *dp;
        while ((dp = readdir(dirp)) != NULL) {
            // reduced verbosity
        }
        closedir(dirp);
    } else {
        modelLogger.error("Le repertoire n'existe pas ou n'est pas accessible");
    }

    // process ASSIMP's root node recursively
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene)
{
    // process each mesh located at the current node
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        // the node object only contains indices to index the actual objects in the scene. 
        // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    
    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    // data to fill
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // walk through each of the mesh's vertices
    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z; 
        vertex.Position = vector;
        
        // normals
        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }
        
        // texture coordinates
        if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x; 
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
            
            // tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;
            
            // bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }
    
    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    
    // process materials
    aiMaterial* material = nullptr;
    
    // Essayer de trouver un matériau avec des textures
    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* mat = scene->mMaterials[i];
        if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0 || 
            mat->GetTextureCount(aiTextureType_SPECULAR) > 0) {
            material = mat;
            break;
        }
    }
    
    // Si aucun matériau avec des textures n'a été trouvé, utiliser le matériau du maillage
    if (!material && mesh->mMaterialIndex >= 0) {
        material = scene->mMaterials[mesh->mMaterialIndex];
    }
    
    if (material) {
        // Afficher les informations sur le matériau
        aiString matName;
        material->Get(AI_MATKEY_NAME, matName);
        modelLogger.debug(std::string("Traitement du matériau: ") + matName.C_Str());
        
        // Afficher le nombre de textures de chaque type
        modelLogger.debug(std::string("Textures trouvées - Diffuse=") + std::to_string(material->GetTextureCount(aiTextureType_DIFFUSE)) +
                  ", Specular=" + std::to_string(material->GetTextureCount(aiTextureType_SPECULAR)) +
                  ", Normal=" + std::to_string(material->GetTextureCount(aiTextureType_NORMALS)) +
                  ", Height=" + std::to_string(material->GetTextureCount(aiTextureType_HEIGHT)) +
                  ", Ambient=" + std::to_string(material->GetTextureCount(aiTextureType_AMBIENT)));

        // Charger les textures en fonction du type
        auto loadAndAddTextures = [&](aiTextureType type, const std::string& typeName) {
            std::vector<Texture> loadedTextures = loadMaterialTextures(material, type, typeName);
            textures.insert(textures.end(), loadedTextures.begin(), loadedTextures.end());
            return loadedTextures;
        };

        // 1. Charger les textures diffuses
        std::vector<Texture> diffuseMaps = loadAndAddTextures(aiTextureType_DIFFUSE, "texture_diffuse");
        
        // 2. Charger les textures spéculaires
        std::vector<Texture> specularMaps = loadAndAddTextures(aiTextureType_SPECULAR, "texture_specular");
        
        // 3. Charger les normales (stockées dans HEIGHT avec Assimp)
        std::vector<Texture> normalMaps = loadAndAddTextures(aiTextureType_HEIGHT, "texture_normal");
        
        // 4. Charger les hauteurs (stockées dans AMBIENT avec Assimp)
        std::vector<Texture> heightMaps = loadAndAddTextures(aiTextureType_AMBIENT, "texture_height");
        
        // 5. Si pas de texture ambiante, utiliser la texture diffuse
        if (material->GetTextureCount(aiTextureType_AMBIENT) == 0 && !diffuseMaps.empty()) {
            modelLogger.debug("Utilisation de la texture diffuse comme ambiante");
            for (auto& tex : diffuseMaps) {
                bool alreadyAdded = false;
                for (const auto& existing : textures) {
                    if (existing.path == tex.path && existing.type == "texture_ambient") {
                        alreadyAdded = true;
                        break;
                    }
                }
                if (!alreadyAdded) {
                    Texture ambientTex = tex;
                    ambientTex.type = "texture_ambient";
                    textures.push_back(ambientTex);
                }
            }
        } else {
            loadAndAddTextures(aiTextureType_AMBIENT, "texture_ambient");
        }
        
        // Afficher la couleur diffuse du matériau
        aiColor3D diffuseColor(0.6f, 0.6f, 0.6f); // Couleur par défaut
        if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor)) {
            modelLogger.debug("Couleur diffuse presente dans le materiau");
        }
        
        // Si aucune texture n'a été chargée, essayer de charger manuellement les textures
        if (textures.empty()) {
            modelLogger.debug("Aucune texture chargee, tentative de chargement manuel...");
            
            // Essayer de charger les textures depuis le répertoire du modèle
            std::string directory = this->directory;
            
            // Vérifier et charger la texture diffuse
            std::string diffusePath = directory + "/diffuse.jpg";
            if (std::ifstream(diffusePath)) {
                Texture diffuseTex;
                diffuseTex.id = Texture2D::Load(diffusePath);
                diffuseTex.type = "texture_diffuse";
                diffuseTex.path = diffusePath;
                textures.push_back(diffuseTex);
                modelLogger.info(std::string("Texture diffuse chargee manuellement: ") + diffusePath);
            }
            
            // Vérifier et charger la texture spéculaire
            std::string specularPath = directory + "/specular.jpg";
            if (std::ifstream(specularPath)) {
                Texture specularTex;
                specularTex.id = Texture2D::Load(specularPath);
                specularTex.type = "texture_specular";
                specularTex.path = specularPath;
                textures.push_back(specularTex);
                modelLogger.info(std::string("Texture speculaire chargee manuellement: ") + specularPath);
            }
            
            // Essayer de charger une texture diffuse par défaut
            std::string texName = "diffuse.jpg";
            std::string fullPath = directory + "/" + texName;
            
            // Vérifier si cette texture est déjà chargée
            bool alreadyLoaded = false;
            for (const auto& tex : textures_loaded) {
                if (tex.path == fullPath) {
                    textures.push_back(tex);
                    alreadyLoaded = true;
                    modelLogger.debug(std::string("Texture deja chargee: ") + fullPath + " (ID: " + std::to_string(tex.id) + ")");
                    break;
                }
            }
            
            if (!alreadyLoaded) {
                modelLogger.debug(std::string("Essai de chargement manuel de la texture: ") + fullPath);
                
                // Vérifier si le fichier existe
                std::ifstream file(fullPath);
                if (file.good()) {
                    file.close();
                    modelLogger.debug(std::string("Fichier de texture trouve: ") + fullPath);
                    unsigned int textureID = Texture2D::Load(fullPath);
                    if (textureID > 0) {
                        Texture texture;
                        texture.id = textureID;
                        texture.type = "texture_diffuse";
                        texture.path = fullPath;
                        textures.push_back(texture);
                        textures_loaded.push_back(texture);
                        modelLogger.info(std::string("Texture chargee avec succes (ID: ") + std::to_string(texture.id) + ")");
                    }
                } else {
                    modelLogger.error(std::string("Impossible de trouver le fichier de texture: ") + fullPath);
                }
            }
        }
    }

    // return a mesh object created from the extracted mesh data
    this->textures_loaded_flag = true;
    return Mesh(vertices, indices, textures);
}

// Variable statique pour suivre les textures déjà chargées
static std::map<std::string, Texture> globalTexturesLoaded;

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
{
    std::vector<Texture> textures;
    
    if (!mat) {
        modelLogger.error("Materiau invalide");
        return textures;
    }
    
    // 1. D'abord, essayer de charger via Assimp
    unsigned int textureCount = mat->GetTextureCount(type);
    modelLogger.debug(std::string("Recherche de ") + typeName + ": " + std::to_string(textureCount) + " textures trouvees dans le materiau");
    
    if (textureCount > 0) {
        for (unsigned int i = 0; i < textureCount; i++) {
            aiString str;
            if (mat->GetTexture(type, i, &str) == AI_SUCCESS) {
                std::string texturePath = str.C_Str();
                modelLogger.debug(std::string("Chemin de texture trouve: ") + texturePath);
                
                // Essayer plusieurs chemins possibles
                std::vector<std::string> possiblePaths = {
                    directory + "/" + texturePath,  // Chemin complet original
                    directory + "/" + texturePath.substr(texturePath.find_last_of("/\\") + 1),  // Juste le nom du fichier
                    texturePath,  // Chemin absolu ou relatif direct
                    directory + "/textures/" + texturePath.substr(texturePath.find_last_of("/\\") + 1),  // Dossier textures/
                    directory + "/" + typeName + "/" + texturePath.substr(texturePath.find_last_of("/\\") + 1)  // Dossier par type
                };
                
                for (const auto& fullPath : possiblePaths) {
                    modelLogger.debug(std::string("Essai de chargement: ") + fullPath);
                    
                    // Vérifier si la texture est déjà chargée
                    bool alreadyLoaded = false;
                    for (const auto& loadedTex : textures_loaded) {
                        if (loadedTex.path == fullPath) {
                            modelLogger.debug(std::string("CACHE ") + typeName + " deja chargee: " + fullPath);
                            textures.push_back(loadedTex);
                            alreadyLoaded = true;
                            break;
                        }
                    }
                    if (alreadyLoaded) continue;
                    
                    // Vérifier si le fichier existe
                    std::ifstream file(fullPath);
                    if (file.good()) {
                        file.close();
                        modelLogger.info(std::string("LOAD ") + typeName + ": " + fullPath);
                        
                        unsigned int textureID = Texture2D::Load(fullPath);
                        
                        if (textureID > 0) {
                            Texture texture;
                            texture.id = textureID;
                            texture.type = typeName;
                            texture.path = fullPath;
                            textures.push_back(texture);
                            textures_loaded.push_back(texture);
                            modelLogger.info(std::string("SUCCESS ") + typeName + " (ID: " + std::to_string(textureID) + ")");
                            return textures;
                        } else {
                            modelLogger.error(std::string("Echec du chargement texture: ") + fullPath);
                        }
                    }
                }
            } else {
                modelLogger.error(std::string("Impossible de recuperer la texture ") + std::to_string(i) + " de type " + typeName);
            }
        }
    }
    
    // 2. Si pas de texture via Assimp, essayer de charger manuellement
    std::vector<std::string> possibleFiles;
    
    // Définir les noms de fichiers possibles en fonction du type de texture
    if (type == aiTextureType_DIFFUSE || type == aiTextureType_AMBIENT) {
        possibleFiles = {"diffuse.jpg", "diffuse.png", "albedo.jpg", "albedo.png", "basecolor.jpg", "basecolor.png", "texture.jpg"};
    } else if (type == aiTextureType_SPECULAR) {
        possibleFiles = {"specular.jpg", "specular.png", "spec.jpg", "spec.png", "metallic.jpg", "metallic.png", "specular_map.jpg"};
    } else {
        return textures;
    }
    
    // Essayer chaque fichier possible dans plusieurs emplacements
    std::vector<std::string> searchDirs = {
        directory,
        directory + "/textures",
        directory + "/Textures",
        directory + "/" + typeName,
        directory + "/" + typeName + "_textures"
    };
    
    for (const auto& searchDir : searchDirs) {
        for (const auto& texName : possibleFiles) {
            std::string fullPath = searchDir + "/" + texName;
            
            // Vérifier si la texture est déjà chargée
            bool alreadyLoaded = false;
            for (const auto& loadedTex : textures_loaded) {
                if (loadedTex.path == fullPath) {
                    textures.push_back(loadedTex);
                    return textures;
                }
            }
            
            // Vérifier si le fichier existe
            std::ifstream file(fullPath);
            if (file.good()) {
                file.close();
                modelLogger.info(std::string("LOAD default ") + typeName + ": " + fullPath);
                
                unsigned int textureID = Texture2D::Load(fullPath);
                
                if (textureID > 0) {
                    Texture texture;
                    texture.id = textureID;
                    texture.type = typeName;
                    texture.path = fullPath;
                    textures.push_back(texture);
                    textures_loaded.push_back(texture);
                    modelLogger.info(std::string("SUCCESS default ") + typeName + " (ID: " + std::to_string(textureID) + ")");
                    return textures;
                } else {
                    modelLogger.error(std::string("Echec chargement texture par defaut: ") + fullPath);
                }
            }
        }
    }
    
    // Si on arrive ici, aucune texture n'a pu être chargée
    modelLogger.error(typeName + ": Aucune texture valide trouvee pour ce type");
    modelLogger.debug(std::string("Repertoire de recherche: ") + directory);
    return textures;
}
