#include "App.h"

int main() {
    App app;
    if (!app.init(1600, 900, "3D-Sandbox")) return -1;
    int ret = app.run();
    app.shutdown();
    return ret;
}