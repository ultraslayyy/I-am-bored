#include <chrono>
#include <windows.h>
#include "application.h"
#include "input.h"
#include "time.h"
#include "../game/game.h"
#include "../platform/d2d_renderer.h"

// No GDIRenderer here *yet*
D2DRenderer renderer;
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
        
        renderer.clear(0, 0, 0);
        game.render(renderer);
        renderer.present();
    }
}