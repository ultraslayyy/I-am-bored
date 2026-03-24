#include <algorithm>
#include <cmath>
#include <cstdio>
#include "../core/input.h"
#include "../core/time.h"
#include "game.h"

int box_width = 100;
int box_height = box_width;

void Game::init(IRenderer* render) {
    renderer = render;
    window_width = renderer->getWidth();
    window_height = renderer->getHeight();
}

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

    x = std::clamp(x, 0.0f, window_width - (float)box_width);
    y = std::clamp(y, 0.0f, window_height - (float)box_height);
}

void Game::render() {
    renderer->drawRect((int)x, (int)y, box_width, box_height, 255, 0, 0);

    char buffer[32];
    sprintf_s(buffer, "FPS: %.0f", Time::fps);
    renderer->drawText(buffer, 10.0f, 10.0f, 20.0f, 255, 255, 255);
}

void Game::onResize(int w, int h) {
    window_width = w;
    window_height = h;
}