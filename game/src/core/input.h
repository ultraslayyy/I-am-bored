#pragma once
#include <array>

enum class Key {
    Left,
    Right,
    Count
};

class Input {
public:
    static void setKey(Key key, bool down);
    static bool isKeyDown(Key key);

private:
    static std::array<bool, (int)Key::Count> keys;
};