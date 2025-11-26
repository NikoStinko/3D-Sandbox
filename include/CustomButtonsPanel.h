#ifndef CUSTOM_BUTTONS_PANEL_H
#define CUSTOM_BUTTONS_PANEL_H

class EditorState;

class CustomButtonsPanel {
public:
    explicit CustomButtonsPanel(EditorState* editorState);

    void draw(bool* open = nullptr);
    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }

private:
    bool m_visible = true;

private:
    EditorState* editor = nullptr;
};

#endif // CUSTOM_BUTTONS_PANEL_H
