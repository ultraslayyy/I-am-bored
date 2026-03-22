#include "input.h"

std::array<bool, (int)Key::Count> Input::keys = {};

void Input::setKey(Key key, bool down) {
    keys[(int)key] = down;
}

bool Input::isKeyDown(Key key) {
    return keys[(int)key];
}