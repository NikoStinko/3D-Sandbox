#ifndef MAP_PANEL_H
#define MAP_PANEL_H

#include <string>
#include <vector>
#include <functional>

class EditorState;
struct SceneSnapshot;

class MapPanel {
public:
    using SaveCallback = std::function<bool(const std::string&)>;
    using LoadCallback = std::function<bool(const std::string&)>;
    using NewCallback = std::function<bool(const std::string&)>;

    MapPanel(const std::string& root,
             SaveCallback saveCb,
             LoadCallback loadCb,
             NewCallback newCb,
             EditorState* editor);

    void draw(bool* open = nullptr);
    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }

private:
    bool m_visible = true;

    void rescan();

private:
    std::string mapsRoot;
    SaveCallback save;
    LoadCallback load;
    NewCallback createNew;
    EditorState* editor;

    std::vector<std::string> files;
    int selected = -1;
    std::string newMapName = "new_map";
    std::string filter;

    void scanFiles();
    std::string selectedPath() const;
    std::string resolvePath(const std::string& name) const;
    std::string displayName(const std::string& path) const;
    std::string pathForCurrentMap() const;
    void selectPath(const std::string& path);
};

#endif // MAP_PANEL_H
