#include <chrono>
#include <windows.h>
#include "application.h"
#include "input.h"
#include "time.h"
#include "../game/game.h"

#if USE_DIRECT2D
#include "../renderer/d2d_renderer.h"
D2DRenderer renderer;
#elif USE_GDI
#include "../renderer/gdi_renderer.h"
GDIRenderer renderer;
#elif USE_OPENGL
#include "../renderer/gl_renderer.h"
GLRenderer renderer;
#endif

Game game;
int width;
int height;

bool Application::init() {
    if (!window.create(800, 600, "Game")) return false;

    window.onResize = [&](int w, int h) {
        width = w;
        height = h;

        renderer.resize(w, h);
        game.onResize(w, h);
    };

    RECT rect;
    GetClientRect(window.getHandle(), &rect);

    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    if (!renderer.init(window.getHandle(), width, height))
        return false;

    return true;
}

void Application::run() {
    game.init(&renderer);

    while (window.isRunning()) {
        window.pollEvents();
        Time::update();

        game.update(Time::deltaTime);
        
        renderer.clear(0, 0, 0);
        game.render();
        renderer.present();
    }
}