#pragma once

#include <stdint.h>

enum ErrorType {
    NO_ERROR,
    STOPPED,
    STACK_UNDERFLOW,
    STACK_OVERFLOW,
    OUT_OF_MEMORY,
    BAD_READ,
    BAD_FETCH,
    UNIMPLEMENTED_INSTRUCTION
};
