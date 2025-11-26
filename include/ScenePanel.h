#ifndef SCENE_PANEL_H
#define SCENE_PANEL_H

class SceneState;

class ScenePanel {
public:
    ScenePanel(SceneState* state);
    void draw(bool* open = nullptr);
    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }

private:
    bool m_visible = true;

private:
    SceneState* scene = nullptr;
};

#endif // SCENE_PANEL_H
