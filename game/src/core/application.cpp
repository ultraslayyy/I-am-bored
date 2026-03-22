#include <chrono>
#include <windows.h>
#include "application.h"
#include "input.h"
#include "time.h"
#include "../game/game.h"

#ifdef DIRECT2D
#include "../platform/d2d_renderer.h"
D2DRenderer renderer;
#elif GDI
#include "../platform/gdi_renderer.h"
GDIRenderer renderer;
#endif
Game game;

bool Application::init() {
    if (!window.create(800, 600, "Game")) return false;

    renderer.init(window.getHandle(), 800, 600);

    return true;
}

void Application::run() {
    while (window.isRunning()) {
        window.pollEvents();
        Time::update();

        game.update(Time::deltaTime);
        game.render(renderer);

        char title[64];
        sprintf_s(title, "FPS: %.0f", Time::fps);
        SetWindowText(window.getHandle(), title);
    }
}