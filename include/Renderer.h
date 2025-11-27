#ifndef RENDERER_H
#define RENDERER_H

class Shader;
class ModelManager;
class Grid;
class Camera;
class EditorState;

class Renderer {
public:
    void render(Shader& shader, ModelManager& modelManager, Grid& grid, const Camera& camera, float deltaTime, const EditorState& editorState);
};

#endif // RENDERER_H