#include "../core/input.h"
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
    renderer.clear(0, 0, 0);
    renderer.drawRect((int)x, 250, 100, 100, 255, 0, 0);
    renderer.present();
}