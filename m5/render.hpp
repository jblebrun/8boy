#include "src/chip8/render.hpp"
#include "gamepad.hpp"

class M5Render : public Render {
    uint16_t* mPixelData;

    TFT_eSprite mSprite = TFT_eSprite(&M5.Lcd);

    M5Gamepad &mGamepad;

    uint8_t mKeymapUD = 0x12;
    uint8_t mKeymapLR = 0x12;
    uint8_t mKeymapAB = 0x12;

    uint8_t mPixelWidth = 4;
    uint8_t mPixelHeight = 4;
    uint8_t mWidth = 64;
    uint8_t mHeight = 32;

    public:
    
    M5Render(M5Gamepad &gamepad);
    ~M5Render();

    void setKeyMap(uint8_t ud, uint8_t lr, uint8_t ab);

    // if it should trigger a collision, return true.
    virtual bool drawPixel(uint8_t x, uint8_t y, bool drawVal);

    // draw the screen
    virtual void render();

    // clear the screen
    virtual void clear();

    virtual void setMode(RenderMode mode);
    

    // SUPER CHIP-8
    
    // scroll the display down the specified number of lines
    virtual void scrollDown(uint8_t amt);
    
    // scroll the display left 4 columns
    virtual void scrollLeft();
    
    // scroll the display right 4 columns
    virtual void scrollRight();


    // Non-drawing rendering
    
    // implementations should make a noise for dur/60 seconds.
    virtual void beep(uint8_t dur);

    // implementations should return a uniform random number from 0 - 0xFF
    virtual uint8_t random();

    // return the button state. Should return a bitmask for the buttons that have 
    // been pressed, in little-endian order for 0-F.
    virtual uint16_t buttons();
};
