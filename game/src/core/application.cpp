#include <chrono>
#include <windows.h>
#include "application.h"
#include "input.h"
#include "renderer.h"
#include "time.h"

Renderer renderer;

bool Application::init() {
    if (!window.create(800, 600, "Game")) return false;

    renderer.init(window.getHandle(), 800, 600);

    return true;
}

void Application::run() {
    while (window.isRunning()) {
        window.pollEvents();

        Time::update();

        update();
        render();

        char title[64];
        sprintf_s(title, "FPS: %.0f", Time::fps);
        SetWindowText(window.getHandle(), title);
    }
}

void Application::update() {
    float speed = 300.0f;

    if (Input::isKeyDown(Key::Left)) {
        x -= speed * Time::deltaTime;
    }
    if (Input::isKeyDown(Key::Right)) {
        x += speed * Time::deltaTime;
    }
}

void Application::render() {
    renderer.clear(0, 0, 0);
    renderer.drawRect((int)x, 250, 100, 100, 255, 0, 0);
    renderer.present();
}