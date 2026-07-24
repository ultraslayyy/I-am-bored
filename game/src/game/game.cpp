#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <cstdio>
#include "../core/input.h"
#include "../core/time.h"
#include "game.h"

#ifdef WIN32
#define iprintf sprintf_s
#else
#define iprintf sprintf
#endif

constexpr int PLAYER_WIDTH  = 100;
constexpr int PLAYER_HEIGHT = 100;

constexpr int GREEN_WIDTH  = 60;
constexpr int GREEN_HEIGHT = 60;

constexpr uint32_t SALT = 0xA5DF3C2B;

void Game::saveHighscore() {
    FILE* f = fopen("hs", "wb");
    if (!f) return;
    
    uint32_t seed = static_cast<uint32_t>(time(nullptr) ^ rand());
    uint32_t obfScore = static_cast<uint32_t>(score) ^ (seed * 2654435761u);

    fwrite(&seed, sizeof(seed), 1, f);
    fwrite(&obfScore, sizeof(obfScore), 1, f);
    fclose(f);
}

void Game::loadHighscore() {
    FILE* f = fopen("hs", "rb");
    if (!f) return;

    uint32_t seed;
    uint32_t obfScore;

    fread(&seed, sizeof(seed), 1, f);
    fread(&obfScore, sizeof(obfScore), 1, f);
    fclose(f);

    highscore = static_cast<int>(obfScore ^ (seed * 2654435761u));
}

float Game::getDistanceToGreen() {
    float playerRight  = x + PLAYER_WIDTH;
    float playerBottom = y + PLAYER_HEIGHT;

    float dx = 0.0f;
    if (playerRight < green_x) {
        dx = green_x - playerRight;
    } else if (x > green_x + GREEN_WIDTH) {
        dx = x - (green_x + GREEN_WIDTH);
    }

    float dy = 0.0f;
    if (playerBottom < green_y) {
        dy = green_y - playerBottom;
    } else if (y > green_y + GREEN_HEIGHT) {
        dy = y - (green_y + GREEN_HEIGHT);
    }

    return std::sqrt(dx * dx + dy * dy);
}

void Game::updateGreenPixelPos() {
    green_u = std::clamp(green_u, 0.0f, 1.0f);
    green_v = std::clamp(green_v, 0.0f, 1.0f);

    green_x = green_u * (window_width  - GREEN_WIDTH);
    green_y = green_v * (window_height - GREEN_HEIGHT);
}

void Game::randomiseGreenBox() {
    float px = (float)(rand() % (window_width  - GREEN_WIDTH));
    float py = (float)(rand() % (window_height - GREEN_HEIGHT));

    green_u = px / (window_width  - GREEN_WIDTH);
    green_v = py / (window_height - GREEN_HEIGHT);

    updateGreenPixelPos();
}

void Game::startNewRound() {
    randomiseGreenBox();

    float distance = getDistanceToGreen();
    float speed = 300.0f;

    time_limit = (distance / speed) * 1.5f + 0.3f;
    // time_limit *= std::max(0.7f, 1.0f - score * 0.02f);
    time_limit = std::max(time_limit, 0.5f);

    timer = time_limit;
}

void Game::init(IRenderer* render) {
    renderer = render;
    window_width = renderer->getWidth();
    window_height = renderer->getHeight();

    srand((unsigned int)time(nullptr));

    loadHighscore();

    updateGreenPixelPos();
}

void Game::update(float dt) {
    float speed = 300.0f;
    float dx = 0.0f;
    float dy = 0.0f;

    if (state == GameState::GameOver) {
        if (Input::isKeyDown(Key::Enter)) {
            state = GameState::Waiting;
            score = 0;
            timer = 0.0f;

            x = window_width / 2.0f;
            y = window_height / 2.0f;

            randomiseGreenBox();
            return;
        }
        return;
    }

    if (Input::isKeyDown(Key::Left) || Input::isKeyDown(Key::A))
        dx -= 1.0f;
    if (Input::isKeyDown(Key::Right) || Input::isKeyDown(Key::D))
        dx += 1.0f;
    if (Input::isKeyDown(Key::Up) || Input::isKeyDown(Key::W))
        dy -= 1.0f;
    if (Input::isKeyDown(Key::Down) || Input::isKeyDown(Key::S))
        dy += 1.0f;

    float length = std::sqrt(dx * dx + dy * dy);
    if (length > 0.0f) {
        dx /= length;
        dy /= length;
    }

    x += dx * speed * dt;
    y += dy * speed * dt;

    x = std::clamp(x, 0.0f, window_width - (float)PLAYER_WIDTH);
    y = std::clamp(y, 0.0f, window_height - (float)PLAYER_HEIGHT);

    if (state == GameState::Playing) {
        timer -= dt;

        if (timer <= 0.0f) {
            timer = 0.0f;
            state = GameState::GameOver;

            if (score > highscore) {
                highscore = score;
                saveHighscore();
            }
        }
    }

    updateGreenPixelPos();

    bool collision =
        x < green_x + GREEN_WIDTH  &&
        x + PLAYER_WIDTH > green_x &&
        y < green_y + GREEN_HEIGHT &&
        y + PLAYER_HEIGHT > green_y;

    if (collision) {
        if (state == GameState::Waiting) {
            state = GameState::Playing;
            score = 0;

            startNewRound();
        } else if (state == GameState::Playing) {
            score++;
            startNewRound();
        }
    }
}

void Game::render() {
    renderer->drawRect((int)green_x, (int)green_y, GREEN_WIDTH, GREEN_HEIGHT, 0, 255, 0);
    renderer->drawRect((int)x, (int)y, PLAYER_WIDTH, PLAYER_HEIGHT, 255, 0, 0);

    char buffer[128];
    iprintf(buffer, "FPS: %.0f", ITime::fps);
    renderer->drawText(buffer, 10.0f, 10.0f, 20.0f, 255, 255, 255);

    iprintf(buffer, "Score %d", score);
    renderer->drawText(buffer, 10, 40, 20, 255, 255, 255);

    if (state == GameState::Playing) {
        iprintf(buffer, "Time: %.2f", timer);
        renderer->drawText(buffer, 10, 70, 20, 255, 255, 255);
    } else if (state == GameState::Waiting) {
        renderer->drawText("Touch green to start", 300, 200, 30, 255, 255, 0);
    } else if (state == GameState::GameOver) {
        renderer->drawText("GAME OVER", 300, 200, 40, 255, 255, 255);

        iprintf(buffer, "Score: %d", score);
        renderer->drawText(buffer, 300, 250, 25, 255, 255, 255);

        iprintf(buffer, "Highscore: %d", highscore);
        renderer->drawText(buffer, 300, 280, 25, 255, 255, 255);

        renderer->drawText("Press Enter to Restart", 300, 320, 20, 200, 200, 200);
    }
}

void Game::onResize(int w, int h) {
    window_width = w;
    window_height = h;

    updateGreenPixelPos();
}