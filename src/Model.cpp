#include "Model.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
// Activation du logging Assimp de base
#define ASSIMP_DO_NOT_USE_GOOGLE_LOGGING

unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma);

Model::Model(std::string const &path, bool gamma) : gammaCorrection(gamma)
{
    loadModel(path);
}

void Model::Draw(Shader &shader)
{
    for(unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shader);
}

void Model::loadModel(std::string const &path)
{
    std::cout << "\nChargement du modele: " << path << std::endl;
    
    // Vérifier si le fichier existe
    std::ifstream file(path);
    if (!file.good()) {
        std::cerr << "ERREUR: Le fichier du modele n'existe pas: " << path << std::endl;
        // Afficher le répertoire de travail actuel
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            std::cout << "Repertoire de travail actuel: " << cwd << std::endl;
        }
        return;
    }
    file.close();
    
    // read file via ASSIMP
    Assimp::Importer importer;
    
    // Logging simple pour Assimp
    std::cout << "\n=== CHARGEMENT DU MODELE ===" << std::endl;
    std::cout << "Chemin du modele: " << path << std::endl;
    
    // Vérifier si le fichier existe et est accessible
    std::ifstream fileCheck(path);
    if (!fileCheck.is_open()) {
        std::cerr << "ERREUR: Impossible d'ouvrir le fichier du modele: " << path << std::endl;
        return;
    }
    fileCheck.close();
    
    // Charger la scène avec des flags supplémentaires pour le débogage
    const aiScene* scene = importer.ReadFile(path, 
        aiProcess_Triangulate | 
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace |
        aiProcess_GenNormals |
        aiProcess_ValidateDataStructure);
        
    // Afficher des informations sur la scène chargée
    if (scene) {
        std::cout << "\n=== INFORMATIONS SUR LE MODELE ===" << std::endl;
        std::cout << "- Nombre de maillages: " << scene->mNumMeshes << std::endl;
        std::cout << "- Nombre de matériaux: " << scene->mNumMaterials << std::endl;
        std::cout << "- Nombre de textures: " << (scene->mTextures ? scene->mNumTextures : 0) << std::endl;
        
        // Afficher les informations sur les matériaux
        for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
            aiMaterial* mat = scene->mMaterials[i];
            aiString name;
            mat->Get(AI_MATKEY_NAME, name);
            std::cout << "\nMateriau " << i << ": " << name.C_Str() << std::endl;
            
            // Compter les textures de chaque type
            std::cout << "  Textures: " << std::endl;
            std::cout << "  - Diffuse: " << mat->GetTextureCount(aiTextureType_DIFFUSE) << std::endl;
            std::cout << "  - Specular: " << mat->GetTextureCount(aiTextureType_SPECULAR) << std::endl;
            std::cout << "  - Normal: " << mat->GetTextureCount(aiTextureType_NORMALS) << std::endl;
            std::cout << "  - Height: " << mat->GetTextureCount(aiTextureType_HEIGHT) << std::endl;
            
            // Afficher les propriétés du matériau
            aiColor3D diffuse, specular, ambient;
            float shininess;
            
            if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS) {
                std::cout << "  - Couleur diffuse: (" << diffuse.r << ", " << diffuse.g << ", " << diffuse.b << ")" << std::endl;
            }
            
            if (mat->Get(AI_MATKEY_COLOR_SPECULAR, specular) == AI_SUCCESS) {
                std::cout << "  - Couleur spéculaire: (" << specular.r << ", " << specular.g << ", " << specular.b << ")" << std::endl;
            }
            
            if (mat->Get(AI_MATKEY_COLOR_AMBIENT, ambient) == AI_SUCCESS) {
                std::cout << "  - Couleur ambiante: (" << ambient.r << ", " << ambient.g << ", " << ambient.b << ")" << std::endl;
            }
            
            if (mat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
                std::cout << "  - Brillance: " << shininess << std::endl;
            }
        }
        
        // Afficher les informations sur les textures intégrées
        if (scene->mTextures) {
            std::cout << "\n=== TEXTURES INTEGREES ===" << std::endl;
            for (unsigned int i = 0; i < scene->mNumTextures; i++) {
                aiTexture* tex = scene->mTextures[i];
                std::cout << "- Texture " << i << ": " << tex->mFilename.C_Str() 
                          << " (" << tex->mWidth << "x" << (tex->mHeight > 0 ? tex->mHeight : tex->mWidth) << ")" << std::endl;
            }
        }
    }
        
    // Vérifier les erreurs
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERREUR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    
    // Récupérer le chemin du répertoire du modèle
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        directory = path.substr(0, lastSlash);
    } else {
        // Si pas de slash, le répertoire est le répertoire courant
        directory = ".";
    }
    
    std::cout << "\n=== CHEMIN DU MODELE ===" << std::endl;
    std::cout << "- Chemin complet: " << path << std::endl;
    std::cout << "- Répertoire: " << directory << std::endl;
    
    // Vérifier si le répertoire existe
    DIR* dir = opendir(directory.c_str());
    if (dir) {
        std::cout << "- Le répertoire existe" << std::endl;
        closedir(dir);
        
        // Lister les fichiers dans le répertoire
        std::cout << "\nFichiers dans le répertoire:" << std::endl;
        DIR *dirp = opendir(directory.c_str());
        struct dirent *dp;
        while ((dp = readdir(dirp)) != NULL) {
            std::cout << "- " << dp->d_name << std::endl;
        }
        closedir(dirp);
    } else {
        std::cerr << "- ERREUR: Le répertoire n'existe pas ou n'est pas accessible" << std::endl;
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
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        
        // Afficher les informations sur le matériau
        aiString matName;
        material->Get(AI_MATKEY_NAME, matName);
        std::cout << "\nTraitement du matériau: " << matName.C_Str() << std::endl;
        
        // Afficher le nombre de textures de chaque type
        unsigned int numDiffuse = material->GetTextureCount(aiTextureType_DIFFUSE);
        unsigned int numSpecular = material->GetTextureCount(aiTextureType_SPECULAR);
        
        // Log texture counts for each type
        std::cout << "- Textures trouvées - "
                  << "Diffuse: " << material->GetTextureCount(aiTextureType_DIFFUSE) << ", "
                  << "Specular: " << material->GetTextureCount(aiTextureType_SPECULAR) << ", "
                  << "Normal: " << material->GetTextureCount(aiTextureType_NORMALS) << ", "
                  << "Height: " << material->GetTextureCount(aiTextureType_HEIGHT) << ", "
                  << "Ambient: " << material->GetTextureCount(aiTextureType_AMBIENT) << std::endl;

        // 1. diffuse maps
        std::cout << "\nRecherche de texture_diffuse..." << std::endl;
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        
        // 2. specular maps
        std::cout << "\nRecherche de texture_specular..." << std::endl;
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        
        // 3. normal maps (using aiTextureType_NORMALS instead of HEIGHT for normals)
        std::cout << "\nRecherche de texture_normal..." << std::endl;
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        
        // 4. height maps (using aiTextureType_HEIGHT for height/displacement maps)
        std::cout << "\nRecherche de texture_height..." << std::endl;
        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        
        // 5. Try loading ambient maps as well (sometimes used for additional textures)
        std::cout << "\nRecherche de texture_ambient..." << std::endl;
        std::vector<Texture> ambientMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_ambient");
        textures.insert(textures.end(), ambientMaps.begin(), ambientMaps.end());
        
        // 5. Vérifier les propriétés du matériau
        aiColor3D color;
        if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
            std::cout << "Couleur diffuse: (" << color.r << ", " << color.g << ", " << color.b << ")" << std::endl;
        }
    }

    // return a mesh object created from the extracted mesh data
    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
{
    std::vector<Texture> textures;
    unsigned int textureCount = mat->GetTextureCount(type);
    
    std::cout << "\n=== RECHERCHE DE TEXTURES " << typeName << " ===" << std::endl;
    std::cout << "Nombre de textures trouvées: " << textureCount << std::endl;
    
    for(unsigned int i = 0; i < textureCount; i++)
    {
        aiString str;
        aiTextureMapping mapping;
        unsigned int uvindex = 0;
        float blend = 1.0f;
        aiTextureOp op = aiTextureOp_Add;
        aiTextureMapMode mapmode = aiTextureMapMode_Wrap;
        
        // Obtenir plus d'informations sur la texture
        if (mat->GetTexture(type, i, &str, &mapping, &uvindex, &blend, &op, &mapmode) == AI_SUCCESS) {
            std::string texturePath = str.C_Str();
            
            // Nettoyer le chemin de la texture
            std::replace(texturePath.begin(), texturePath.end(), '\\', '/');
            
            std::cout << "\nDétails de la texture #" << i << ":" << std::endl;
            std::cout << "- Chemin original: " << texturePath << std::endl;
            std::cout << "- Type de mapping: " << mapping << std::endl;
            std::cout << "- Index UV: " << uvindex << std::endl;
            std::cout << "- Intensité: " << blend << std::endl;
            std::cout << "- Opération: " << op << std::endl;
            std::cout << "- Mode de mapping: " << mapmode << std::endl;
            
            // Vérifier si le chemin est absolu ou relatif
            bool isAbsolutePath = (texturePath.find('/') == 0) || 
                                (texturePath.find(':') != std::string::npos) ||
                                (texturePath.find("..") == 0);
            
            std::string fullPath;
            if (isAbsolutePath) {
                fullPath = texturePath;
                std::cout << "- Chemin détecté comme absolu: " << fullPath << std::endl;
            } else {
                // Construire le chemin relatif au répertoire du modèle
                fullPath = this->directory + '/' + texturePath;
                std::cout << "- Chemin construit: " << fullPath << std::endl;
            }
            
            // Vérifier si le fichier existe
            std::ifstream fileTest(fullPath);
            if (!fileTest.good()) {
                std::cerr << "ERREUR: Le fichier de texture n'existe pas: " << fullPath << std::endl;
                
                // Essayer avec juste le nom du fichier (sans le chemin)
                std::string fileName = texturePath.substr(texturePath.find_last_of("/\\") + 1);
                std::string altPath = this->directory + '/' + fileName;
                
                std::cout << "Essai avec le nom de fichier uniquement: " << altPath << std::endl;
                
                std::ifstream altFile(altPath);
                if (altFile.good()) {
                    fullPath = altPath;
                    std::cout << "Texture trouvée avec le chemin alternatif" << std::endl;
                } else {
                    std::cerr << "ERREUR: Impossible de trouver la texture: " << fullPath << std::endl;
                    continue;
                }
            }
            
            // Vérifier si la texture a déjà été chargée
            bool skip = false;
            for(unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    std::cout << "- Texture déjà chargée, utilisation de la copie existante" << std::endl;
                    break;
                }
            }
            
            if(!skip)
            {
                // Si la texture n'a pas été chargée, la charger
                std::cout << "- Chargement de la texture..." << std::endl;
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory, this->gammaCorrection);
                if (texture.id != 0) {  // Vérifier si le chargement a réussi
                    texture.type = typeName;
                    texture.path = str.C_Str();
                    textures.push_back(texture);
                    textures_loaded.push_back(texture);
                    std::cout << "- Texture chargée avec succès (ID: " << texture.id << ")" << std::endl;
                } else {
                    std::cerr << "- ERREUR: Échec du chargement de la texture" << std::endl;
                }
            }
        }
        else
        {
            std::cerr << "Erreur lors de la lecture de la texture #" << i << " de type " << typeName << std::endl;
            continue;
        }
    }
    return textures;
}

unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma)
{
    std::string filename = std::string(path);
    
    // Nettoyer le chemin des retours à la ligne et des backslashes
    filename.erase(std::remove(filename.begin(), filename.end(), '\n'), filename.end());
    filename.erase(std::remove(filename.begin(), filename.end(), '\r'), filename.end());
    std::replace(filename.begin(), filename.end(), '\\', '/');
    
    // Afficher des informations de débogage
    std::cout << "\n=== TENTATIVE DE CHARGEMENT DE TEXTURE ===" << std::endl;
    std::cout << "- Chemin fourni: " << filename << std::endl;
    std::cout << "- Répertoire de base: " << directory << std::endl;
    
    // Vérifier si le chemin est absolu ou relatif
    bool isAbsolutePath = (filename.find('/') == 0) || 
                         (filename.find(':') != std::string::npos) ||
                         (filename.find("..") == 0);
    
    std::string fullPath;
    if (isAbsolutePath) {
        fullPath = filename;
        std::cout << "- Chemin détecté comme absolu: " << fullPath << std::endl;
    } else {
        // Construire le chemin relatif au répertoire du modèle
        fullPath = directory + '/' + filename;
        std::cout << "- Chemin construit: " << fullPath << std::endl;
    }
    
    // Vérifier si le fichier existe
    std::ifstream file(fullPath);
    if (!file.good()) {
        std::cerr << "ERREUR: Le fichier de texture n'existe pas: " << fullPath << std::endl;
        
        // Essayer avec juste le nom du fichier (sans le chemin)
        std::string fileName = filename.substr(filename.find_last_of("/\\") + 1);
        std::string altPath = directory + '/' + fileName;
        
        std::cout << "Essai avec le nom de fichier uniquement: " << altPath << std::endl;
        
        std::ifstream altFile(altPath);
        if (altFile.good()) {
            fullPath = altPath;
            std::cout << "Texture trouvée avec le chemin alternatif" << std::endl;
        } else {
            std::cerr << "ERREUR: Impossible de trouver la texture: " << fullPath << std::endl;
            return 0;
        }
    }
    file.close();

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);  // Important pour les textures OpenGL
    
    std::cout << "Tentative de chargement de la texture: " << fullPath << std::endl;
    unsigned char *data = stbi_load(fullPath.c_str(), &width, &height, &nrComponents, 0);
    
    if (!data) {
        std::cerr << "ERREUR: Impossible de charger la texture: " << stbi_failure_reason() << std::endl;
        return 0;
    }
    if (data)
    {
        std::cout << "Texture chargée avec succès - " 
                  << "Largeur: " << width << ", Hauteur: " << height 
                  << ", Canaux: " << nrComponents << std::endl;
                  
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cerr << "ERREUR: Impossible de charger la texture à l'emplacement: " << filename << std::endl;
        std::cerr << "Raison: " << stbi_failure_reason() << std::endl;
        stbi_image_free(data);
        return 0;  // Retourne 0 si le chargement échoue
    }

    return textureID;
}
