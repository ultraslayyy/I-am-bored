#pragma once
#include "../core/renderer.h"

class Game {
public:
    void init(IRenderer* render);
    void update(float dt);
    void render();
    void onResize(int w, int h);

private:
    IRenderer* renderer = nullptr;

    int window_height;
    int window_width;

    float x = 350.0f;
    float y = 250.0f;
};