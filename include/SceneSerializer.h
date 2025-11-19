#ifndef SCENE_SERIALIZER_H
#define SCENE_SERIALIZER_H

#include <optional>
#include <string>

#include "SceneData.h"

class SceneSerializer {
public:
    static bool Save(const std::string& path, const SceneSnapshot& snapshot);
    static std::optional<SceneSnapshot> Load(const std::string& path);
};

#endif // SCENE_SERIALIZER_H
