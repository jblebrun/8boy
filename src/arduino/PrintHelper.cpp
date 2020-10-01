#include "PrintHelper.hpp"


void PrintHelper::handleCommand(Format f, va_list *src) {
    // If it's in the ASCII range, print a a character.
    if (f < 256) {
        mPrint.write(f);
        return;
    }
    switch(f) {
        // Have to use int, not uint8_t/uint16_t, because va_list 
        // undergoes default argument promotion.
        case D8: handleX<int>(2, src, DEC); break;
        case D12: handleX<int>(3, src, DEC); break;
        case D16: handleX<int>(4, src, DEC); break;
        case X8: handleX<int>(2, src, HEX); break;
        case X12: handleX<int>(3, src, HEX); break;
        case X16: handleX<int>(4, src, HEX); break;
        case FS: handlePrint<__FlashStringHelper*>(src); break;
        case AX8: handleArray<uint8_t*>(2, src, HEX); break;
        case AX12: handleArray<uint16_t*>(3, src, HEX); break;
        case AX16: handleArray<uint16_t*>(4, src, HEX); break;
    }
}

void PrintHelper::printfs(Format f, ...) {
    va_list(argp);
    va_start(argp, f);

    while (f != DONE) {
        handleCommand(f, &argp);
        int fi = va_arg(argp, int);
        f = (Format)fi;
    }
    va_end(argp);
}

template<typename T>
void PrintHelper::printPadded(T w, uint8_t nybbles, int fmt) {
    if(fmt == HEX) {
        for(int i = nybbles - 1; i >= 0; i--) {
            mPrint.print((w >> i*4) & 0xF, fmt);
        }
    } else {
        mPrint.print(w, fmt);
    }
}

template<typename T>
void PrintHelper::printArray(T a, int size, uint8_t nybbles, int fmt) {
    for(int i = 0; i < size; i++) {
        printPadded(a[i], nybbles, fmt);
        mPrint.write(' ');
    }
}

template<typename T>
void PrintHelper::handleX(uint8_t nybbles, va_list *src, int fmt) {
    T n = va_arg(*src, int);
    printPadded(n, nybbles, fmt);
}

template<typename T>
void PrintHelper::handlePrint(va_list *src) {
    T fs = va_arg(*src, T);
    mPrint.print(fs);
}

template<typename T>
void PrintHelper::handleArray(uint8_t nybbles, va_list *src, int fmt) {
    T a = va_arg(*src, T);
    int count = va_arg(*src, int);
    printArray(a, count, nybbles, fmt);
}
