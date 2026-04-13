#include <chrono>
#include "time.h"

static auto lastTime = std::chrono::high_resolution_clock::now();

float ITime::deltaTime = 0.0f;
float ITime::fps = 0.0f;

float ITime::fpsTimer = 0.0f;
int ITime::frameCount = 0;

void ITime::update() {
    using clock = std::chrono::high_resolution_clock;

    auto now = clock::now();
    deltaTime = std::chrono::duration<float>(now - lastTime).count();
    lastTime = now;

    fpsTimer += deltaTime;
    frameCount++;

    if (fpsTimer >= 1.0f) {
        fps = frameCount / fpsTimer;
        fpsTimer = 0.0f;
        frameCount = 0;
    }
}