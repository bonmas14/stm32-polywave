#pragma once

#include <stddef.h>
#include <stdint.h>

enum {
    TRIANGE,
    SAW,
    SQUARE
};


uint8_t osc_generate(uint32_t osc_enum, uint32_t freq, uint64_t step);
