#include "Texture.h"
#include "stb_image.h"
#include <algorithm>
#include <iostream>

std::map<std::string, GLuint> Texture2D::cache;
ComponentLogger Texture2D::logger("Texture");

GLuint Texture2D::Load(const std::string &fullPath, bool flipY, Format fmt)
{
    if (fullPath.empty()) {
        logger.error("Chemin vide pour la texture");
        return 0;
    }

    auto it = cache.find(fullPath);
    if (it != cache.end()) {
        logger.debug(std::string("Cache hit: ") + fullPath);
        return it->second;
    }

    stbi_set_flip_vertically_on_load(flipY);
    int w=0,h=0,n=0;
    unsigned char *data = stbi_load(fullPath.c_str(), &w, &h, &n, 0);
    if (!data) {
        logger.error(std::string("stbi_load a échoué: ") + fullPath + " | " + (stbi_failure_reason()?stbi_failure_reason():""));
        return 0;
    }

    GLenum internalFormat = GL_RGB;
    GLenum format = GL_RGB;
    if (n == 1) { internalFormat = format = GL_RED; }
    else if (n == 3) { internalFormat = format = GL_RGB; }
    else if (n == 4) { internalFormat = format = GL_RGBA; }

    GLuint id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        logger.error("OpenGL erreur lors de la creation texture: code=" + std::to_string(err));
        glDeleteTextures(1, &id);
        return 0;
    }

    cache[fullPath] = id;
    logger.info("Texture chargée: " + fullPath + " (" + std::to_string(w) + "x" + std::to_string(h) + ", channels=" + std::to_string(n) + ")");
    return id;
}

void Texture2D::ClearCache()
{
    for (auto &p : cache) {
        if (p.second) glDeleteTextures(1, &p.second);
    }
    cache.clear();
}
