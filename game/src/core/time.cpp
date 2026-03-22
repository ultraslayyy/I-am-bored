#include <chrono>
#include "time.h"

static auto lastTime = std::chrono::high_resolution_clock::now();

float Time::deltaTime = 0.0f;
float Time::fps = 0.0f;

float Time::fpsTimer = 0.0f;
int Time::frameCount = 0;

void Time::update() {
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