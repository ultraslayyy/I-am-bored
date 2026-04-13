#pragma once

class ITime {
public:
    static void update();

    static float deltaTime;
    static float fps;

private:
    static float fpsTimer;
    static int frameCount;
};