#pragma once
#include "../core/renderer.h"

enum class GameState {
    Waiting,
    Playing,
    GameOver
};

class Game {
public:
    void init(IRenderer* render);
    void update(float dt);
    void render();
    void onResize(int w, int h);

private:
    float getDistanceToGreen();
    void updateGreenPixelPos();
    void randomiseGreenBox();
    void startNewRound();
    void saveHighscore();
    void loadHighscore();

    IRenderer* renderer = nullptr;

    int window_height;
    int window_width;

    float x = 350.0f;
    float y = 250.0f;

    GameState state = GameState::Waiting;

    int score = 0;
    int highscore = 0;

    float timer = 0.0f;
    float time_limit = 0.0f;

    float green_u    = 0.75f; // 0 left, 1 right
    float green_v    = 0.5f; // 0 top, 1 bottom
    float green_x = 0.0f;
    float green_y = 0.0f;
};