#pragma once

#include <stdarg.h>
#include <Arduino.h>

enum Format {
    // Terminal for the printfs functin.
    DONE,
    // The next value should be an 8-bit value. It will be printed in decimal.
    D8 = 1000,
    // The next value should be a 16-bit value, 3 nybbles will be printed in
    // decimalx.
    D12,
    // The next value should be a 16-bit value, 4 nybbles will be printed in
    // decimal.
    D16,
    // The next value should be an 8-bit value. It will be printed in hex.
    X8,
    // The next value should be a 16-bit value, 3 nybbles will be printed in
    // hex.
    X12,
    // The next value should be a 16-bit value, 4 nybbles will be printed in
    // hex.
    X16,
    // Print a flash string (use with F() macro strings).
    FS,
    // The next value should be an array of 8-bit values. The will be printed 
    // in hex, with a space following each one.
    AX8,
    // The next value should be an array of 8-bit values. The will be printed 
    // in hex, with a space following each one.
    AX12,
    // The next value should be an array of 8-bit values. The will be printed 
    // in hex, with a space following each one.
    AX16,
};

class PrintHelper {
    const Print &mPrint;
        
    // A helper to print an array of numbers of the specified nybble size.
    template<typename T>
    void printArray(T a, int size, uint8_t nybbles, int fmt=HEX);

    // Print hex numbers with 0 padding up to the specified number of nybbles.
    template<typename T>
    void printPadded(T w, uint8_t nybbles, int fmt=HEX);

    // Handle format enums for printing single hex numbers.
    template<typename T>
    void handleX(uint8_t nybbles, va_list *src, int fmt=HEX);
    
    // Handle format enums for printing hex arrays.
    template<typename T>
    void handleArray(uint8_t nybbles, va_list *src, int fmt=HEX);
    
    // Handle format enums for printing regular strings.
    template<typename T>
    void handlePrint(va_list *src);

    inline void space() { mPrint.write(' '); }

    void handleCommand(Format f, va_list *src);

    public:
        PrintHelper(const Print &print = Serial) : mPrint(print) {}

    public:
        void printfs(Format f, ...);

};


