#include "Window.h"
#include <GLFW/glfw3.h>
#include <iostream>

Window::Window() = default;
Window::~Window() { destroy(); }

bool Window::create(unsigned int width, unsigned int height, const char* title) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    return true;
}

void Window::destroy() {
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

bool Window::shouldClose() const { return window && glfwWindowShouldClose(window); }
void Window::swapBuffers() { if (window) glfwSwapBuffers(window); }
void Window::pollEvents() { glfwPollEvents(); }