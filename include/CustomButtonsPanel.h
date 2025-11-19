#ifndef CUSTOM_BUTTONS_PANEL_H
#define CUSTOM_BUTTONS_PANEL_H

class EditorState;

class CustomButtonsPanel {
public:
    explicit CustomButtonsPanel(EditorState* editorState);

    void draw(bool* open);

private:
    EditorState* editor = nullptr;
};

#endif // CUSTOM_BUTTONS_PANEL_H
