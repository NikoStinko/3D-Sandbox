#include "Grid.h"
#include "Shader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <vector>

GridRenderer::GridRenderer()
{
    buildGrid(20, 1.0f);
}

GridRenderer::~GridRenderer()
{
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
}

void GridRenderer::buildGrid(int halfSize, float step)
{
    // Each line segment has two vertices, each vertex has position (x,y,z) and color (r,g,b)
    std::vector<float> data;
    auto push_line = [&](glm::vec3 a, glm::vec3 b, glm::vec3 color){
        data.push_back(a.x); data.push_back(a.y); data.push_back(a.z);
        data.push_back(color.r); data.push_back(color.g); data.push_back(color.b);
        data.push_back(b.x); data.push_back(b.y); data.push_back(b.z);
        data.push_back(color.r); data.push_back(color.g); data.push_back(color.b);
    };

    // Grid lines on XZ plane at y=0
    const glm::vec3 gridColor(0.3f, 0.3f, 0.3f);
    const glm::vec3 xColor(1.0f, 0.2f, 0.2f); // X axis (red)
    const glm::vec3 zColor(0.2f, 0.2f, 1.0f); // Z axis (blue)
    const glm::vec3 yColor(0.2f, 1.0f, 0.2f); // Y axis (green)

    for (int i = -halfSize; i <= halfSize; ++i) {
        float x = i * step;
        float z = i * step;
        // Lines parallel to Z (varying X)
        push_line(glm::vec3(x, 0.0f, -halfSize * step), glm::vec3(x, 0.0f, halfSize * step), i == 0 ? xColor : gridColor);
        // Lines parallel to X (varying Z)
        push_line(glm::vec3(-halfSize * step, 0.0f, z), glm::vec3(halfSize * step, 0.0f, z), i == 0 ? zColor : gridColor);
    }

    // Y axis line
    push_line(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, halfSize * step, 0.0f), yColor);

    vertexCount = static_cast<GLsizei>(data.size() / 6);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

    // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    // color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
}

void GridRenderer::draw(Shader &shader, const glm::mat4 &view, const glm::mat4 &proj)
{
    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", proj);
    shader.setMat4("model", glm::mat4(1.0f));
    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, vertexCount);
    glBindVertexArray(0);
}
