#ifndef UI_MODEL_BROWSER_H
#define UI_MODEL_BROWSER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <iostream>
#include <algorithm>

#include <GLFW/glfw3.h>

#include "ModelManager.h"
#include "Log.h"

class UiModelBrowser {
public:
    UiModelBrowser(ModelManager* mgr, const std::string& rootDir)
        : manager(mgr), root(rootDir), visible(false), selected(0) {
        loadFiles();
        refreshMatches();
        lastKeyState.clear();
    }

    void toggle() { visible = !visible; dirty = true; }
    bool isVisible() const { return visible; }

    void handleInput(GLFWwindow* window) {
        if (!visible) return;
        // Navigation
        onKeyEdge(window, GLFW_KEY_UP, [&]{ if (selected > 0) { selected--; dirty = true; } });
        onKeyEdge(window, GLFW_KEY_DOWN, [&]{ if (selected + 1 < (int)matches.size()) { selected++; dirty = true; } });
        onKeyEdge(window, GLFW_KEY_BACKSPACE, [&]{ if (!query.empty()) { query.pop_back(); refreshMatches(); dirty = true; } });
        onKeyEdge(window, GLFW_KEY_ENTER, [&]{ loadSelection(); });
        onKeyEdge(window, GLFW_KEY_KP_ENTER, [&]{ loadSelection(); });

        // Simple character input (A-Z, 0-9, ., _, -, /)
        for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_Z; ++key) {
            if (isPrintableKey(key) && keyEdge(window, key)) {
                char c = translateKey(window, key);
                if (c != '\0') {
                    query.push_back(c);
                    refreshMatches();
                    dirty = true;
                }
            }
        }

        if (dirty) dumpToConsole();
        dirty = false;
    }

private:
    ModelManager* manager;
    std::string root;
    bool visible;
    bool dirty = false;
    std::string query;
    std::vector<std::string> files;      // all candidate model files
    std::vector<std::string> matches;    // filtered by query
    int selected;
    std::unordered_map<int,bool> lastKeyState;
    ComponentLogger logger{"UI"};

    void loadFiles() {
        files.clear();
        namespace fs = std::filesystem;
        std::error_code ec;
        if (!fs::exists(root, ec)) return;
        for (auto& p : fs::recursive_directory_iterator(root, ec)) {
            if (p.is_regular_file()) {
                auto ext = p.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                if (ext == ".obj" || ext == ".fbx" || ext == ".dae" || ext == ".gltf" || ext == ".glb") {
                    files.push_back(p.path().string());
                }
            }
        }
        std::sort(files.begin(), files.end());
    }

    static std::string lower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }

    void refreshMatches() {
        matches.clear();
        if (query.empty()) {
            matches = files;
        } else {
            auto q = lower(query);
            for (auto& f : files) {
                if (lower(f).find(q) != std::string::npos) matches.push_back(f);
            }
        }
        if (selected >= (int)matches.size()) selected = (int)matches.size() - 1;
        if (selected < 0) selected = 0;
    }

    void dumpToConsole() {
        std::cout << "\n=== Model Browser (type to filter, UP/DOWN select, ENTER=OK, BACKSPACE=del, E=close) ===\n";
        std::cout << "Root: " << root << " | Query: '" << query << "'\n";
        int show = std::min((int)matches.size(), 15);
        for (int i = 0; i < show; ++i) {
            std::cout << (i == selected ? "> " : "  ") << matches[i] << "\n";
        }
        if ((int)matches.size() > show) std::cout << "... (" << matches.size()-show << " more)\n";
    }

    void loadSelection() {
        if (matches.empty()) return;
        const std::string& path = matches[selected];
        logger.info(std::string("UI loading model: ") + path);
        if (manager) manager->addModel(path);
        visible = false;
    }

    bool keyEdge(GLFWwindow* window, int key) {
        int state = glfwGetKey(window, key);
        bool pressed = (state == GLFW_PRESS);
        bool last = lastKeyState[key];
        lastKeyState[key] = pressed;
        return pressed && !last;
    }

    template<typename F>
    void onKeyEdge(GLFWwindow* window, int key, F&& f) {
        if (keyEdge(window, key)) f();
    }

    static bool isPrintableKey(int key) {
        return (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) ||
               (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) ||
               key == GLFW_KEY_SPACE || key == GLFW_KEY_PERIOD || key == GLFW_KEY_MINUS || key == GLFW_KEY_SLASH || key == GLFW_KEY_BACKSLASH || key == GLFW_KEY_APOSTROPHE || key == GLFW_KEY_SEMICOLON || key == GLFW_KEY_EQUAL;
    }

    static char translateKey(GLFWwindow* window, int key) {
        bool shift = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
        if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
            char c = 'a' + (key - GLFW_KEY_A);
            return shift ? (char)toupper(c) : c;
        }
        if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) {
            return '0' + (key - GLFW_KEY_0);
        }
        if (key == GLFW_KEY_SPACE) return ' ';
        if (key == GLFW_KEY_PERIOD) return '.';
        if (key == GLFW_KEY_MINUS) return '-';
        if (key == GLFW_KEY_SLASH) return '/';
        if (key == GLFW_KEY_BACKSLASH) return '/';
        if (key == GLFW_KEY_EQUAL) return '=';
        if (key == GLFW_KEY_SEMICOLON) return ';';
        if (key == GLFW_KEY_APOSTROPHE) return '\'';
        return '\0';
    }
};

#endif // UI_MODEL_BROWSER_H
