#ifndef GRID_H
#define GRID_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class Shader;

class Grid {
public:
    Grid();
    ~Grid();

    void draw(Shader &shader, const glm::mat4 &view, const glm::mat4 &proj);

private:
    GLuint vao = 0;
    GLuint vbo = 0;
    GLsizei vertexCount = 0;

    void buildGrid(int halfSize = 10, float step = 1.0f);
};

#endif // GRID_H
