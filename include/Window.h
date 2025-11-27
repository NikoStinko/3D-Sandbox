#ifndef WINDOW_H
#define WINDOW_H

struct GLFWwindow;
class Window {
public:
    Window();
    ~Window();

    bool create(unsigned int width, unsigned int height, const char* title);
    void destroy();
    bool shouldClose() const;
    void swapBuffers();
    void pollEvents();
    GLFWwindow* getGLFW() const { return window; }

private:
    GLFWwindow* window = nullptr;
};

#endif // WINDOW_H