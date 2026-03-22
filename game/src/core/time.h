#pragma once

class Time {
public:
    static void update();

    static float deltaTime;
    static float fps;

private:
    static float fpsTimer;
    static int frameCount;
};