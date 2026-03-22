#include <algorithm>
#include <cmath>
#include <cstdio>
#include "../core/input.h"
#include "../core/time.h"
#include "game.h"

void Game::update(float dt) {
    float speed = 300.0f;
    float dx = 0.0f;
    float dy = 0.0f;

    if (Input::isKeyDown(Key::Left))  dx -= 1.0f;
    if (Input::isKeyDown(Key::Right)) dx += 1.0f;
    if (Input::isKeyDown(Key::Up))    dy -= 1.0f;
    if (Input::isKeyDown(Key::Down))  dy += 1.0f;

    float length = std::sqrt(dx*dx + dy*dy);
    if (length > 0.0f) {
        dx /= length;
        dy /= length;
    }

    x += dx * speed * dt;
    y += dy * speed * dt;

    x = std::clamp(x, 0.0f, 685.0f); // 800 - box size - 15
    y = std::clamp(y, 0.0f, 465.0f); // 700 - box size - 35 ^ hardcoded for now
}

void Game::render(IRenderer& renderer) {
    renderer.drawRect((int)x, (int)y, 100, 100, 255, 0, 0);

    char buffer[32];
    sprintf_s(buffer, "FPS: %.0f", Time::fps);
    renderer.drawText(buffer, 10.0f, 10.0f, 20.0f, 255, 255, 255);
}