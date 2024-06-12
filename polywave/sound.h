#pragma once

#include <stddef.h>
#include <stdint.h>

enum {
    TRIANGE,
    SAW,
    SQUARE
};

void osc_tick(void);
uint8_t osc_generate(uint32_t osc_enum, int32_t freq);
