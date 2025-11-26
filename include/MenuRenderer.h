#ifndef MENU_RENDERER_H
#define MENU_RENDERER_H

#include "EditorState.h"
#include <glm/glm.hpp>
#include <string>

class MenuRenderer {
public:
    static void drawRadialMenu(const EditorState& editorState, const glm::vec2& center, float radius);
    static void drawPauseMenu(const EditorState& editorState);
    static RadialMenuItem handleRadialMenuClick(const EditorState& editorState, const glm::vec2& clickPos, const glm::vec2& center, float radius);
    static PauseMenuItem handlePauseMenuClick(const EditorState& editorState, const glm::vec2& clickPos);
    
private:
    static void drawRadialButton(const glm::vec2& center, float radius, float angle, const std::string& label, bool selected);
    static void drawPauseButton(const glm::vec2& position, const glm::vec2& size, const std::string& label, bool selected);
    
    static glm::vec2 getRadialPosition(const glm::vec2& center, float radius, float angle);
    static RadialMenuItem getRadialItemFromAngle(float angle);
    static PauseMenuItem getPauseItemFromPosition(const glm::vec2& position);
};

#endif // MENU_RENDERER_H