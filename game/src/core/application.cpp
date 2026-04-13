#include <chrono>
#ifdef WIN32
#include <windows.h>
#endif
#include "application.h"
#include "input.h"
#include "time.h"
#include "../game/game.h"

#if USE_DIRECT2D
#include "../renderer/d2d_renderer.h"
D2DRenderer renderer;
#elif USE_DIRECT3D11
#include "../renderer/d3d11_renderer.h"
D3D11Renderer renderer;
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
    width = 800;
    height = 600;

    if (!window.create(width, height, "Game")) return false;

    window.onResize = [&](int w, int h) {
        width = w;
        height = h;

        renderer.resize(w, h);
        game.onResize(w, h);
    };

#ifdef WIN32
    RECT rect;
    GetClientRect(window.getHandle(), &rect);

    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
#endif

    if (!renderer.init(&window, width, height))
        return false;

    return true;
}

void Application::run() {
    game.init(&renderer);

    while (window.isRunning()) {
        window.pollEvents();
        ITime::update();

        game.update(ITime::deltaTime);
        
        renderer.clear(0, 0, 0);
        game.render();
        renderer.present();
    }
}