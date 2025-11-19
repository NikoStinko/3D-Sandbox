#ifndef SCENE_PANEL_H
#define SCENE_PANEL_H

class SceneState;

class ScenePanel {
public:
    ScenePanel(SceneState* state);
    void draw(bool* open);

private:
    SceneState* scene = nullptr;
};

#endif // SCENE_PANEL_H
