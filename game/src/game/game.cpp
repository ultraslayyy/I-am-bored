#include <cstdio>
#include "../core/input.h"
#include "../core/time.h"
#include "game.h"

void Game::update(float dt) {
    float speed = 300.0f;

    if (Input::isKeyDown(Key::Left)) {
        x -= speed * dt;
    }

    if (Input::isKeyDown(Key::Right)) {
        x += speed * dt;
    }
}

void Game::render(IRenderer& renderer) {
    renderer.drawRect((int)x, 250, 100, 100, 255, 0, 0);

    char buffer[32];
    sprintf_s(buffer, "FPS: %.0f", Time::fps);
    renderer.drawText(buffer, 10.0f, 10.0f, 20.0f, 255, 255, 255);
}