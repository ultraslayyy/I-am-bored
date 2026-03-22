#pragma once
#include "../core/renderer.h"

class Game {
public:
    void update(float dt);
    void render(IRenderer& renderer);

private:
    float x = 350.0f;
};