#pragma once
#include <array>

enum class Key {
    A,
    D,
    S,
    W,
    Left,
    Right,
    Up,
    Down,
    Enter,
    Count
};

class Input {
public:
    static void setKey(Key key, bool down);
    static bool isKeyDown(Key key);

private:
    static std::array<bool, (int)Key::Count> keys;
};