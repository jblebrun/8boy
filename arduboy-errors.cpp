#include "arduboy-errors.hpp"

ArduboyErrors::ArduboyErrors(Arduboy2 &boy) : mBoy(boy) {}

void ArduboyErrors::stackUnderflow(uint16_t addr) {
    mBoy.setCursor(0,0);
    mBoy.print(F("UNDERFLOW "));
    mBoy.print(addr, HEX);
}
void ArduboyErrors::stackOverflow(uint16_t addr) {
    mBoy.setCursor(0,0);
    mBoy.print(F("OVERFLOW "));
    mBoy.print(addr, HEX);
}
void ArduboyErrors::oom(uint16_t addr) {
    mBoy.setCursor(0,0);
    mBoy.print(F("OOM "));
    mBoy.print(addr, HEX);
}
void ArduboyErrors::badread(uint16_t addr) {
    mBoy.setCursor(0,0);
    mBoy.print(F("BAD READ "));
    mBoy.print(addr, HEX);
}

void ArduboyErrors::unimpl(uint16_t addr, uint16_t inst) {
    mBoy.setCursor(0,0);
    mBoy.print(F("UNIMPL "));
    mBoy.print(addr, HEX);
    mBoy.print(F(":"));
    mBoy.print(inst, HEX);
}
