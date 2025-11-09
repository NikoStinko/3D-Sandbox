#ifndef TEXTURE2D_H
#define TEXTURE2D_H

#include <GL/glew.h>
#include <string>
#include <map>
#include "Log.h"

class Texture2D {
public:
    enum class Format { Auto, SRGB, RGB, RGBA };

    static GLuint Load(const std::string &fullPath, bool flipY = true, Format fmt = Format::Auto);
    static void ClearCache();

private:
    static std::map<std::string, GLuint> cache;
    static ComponentLogger logger;
};

#endif
