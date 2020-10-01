#pragma once

#define GAMEPAD_UP 0x01
#define GAMEPAD_DOWN 0x02
#define GAMEPAD_LEFT 0x04
#define GAMEPAD_RIGHT 0x08
#define GAMEPAD_A 0x10
#define GAMEPAD_B 0x20
#define GAMEPAD_SEL 0x40
#define GAMEPAD_STA 0x80

class M5Gamepad {
    uint8_t mPressed = 0;
    uint8_t mJustPressed = 0;
    uint8_t mJustReleased = 0;

    public:
    void poll();

    bool pressed(uint8_t which);
    bool justPressed(uint8_t which);
};
