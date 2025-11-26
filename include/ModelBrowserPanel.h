#ifndef MODEL_BROWSER_PANEL_H
#define MODEL_BROWSER_PANEL_H

#include <string>
#include <vector>
#include <filesystem>
#include <memory>

class ModelManager;

class ModelBrowserPanel {
public:
    ModelBrowserPanel(ModelManager* mgr, const std::string& rootDir);
    void draw(bool* open = nullptr);
    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }

private:
    bool m_visible = true;

private:
    ModelManager* manager;
    std::string root;
    std::string filter;
    std::vector<std::string> files;
    int selected = 0;

    void scanFiles();
    bool exists(const std::string& path) const;
};

#endif // MODEL_BROWSER_PANEL_H
