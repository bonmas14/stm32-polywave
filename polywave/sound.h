#pragma once

#include <stddef.h>
#include <stdint.h>

enum {
    TRIANGE,
    SAW,
    SQUARE
};

typedef struct {
    uint8_t osc_enum;

    uint64_t time;

    uint32_t freq;

    int32_t phase;
} osc_t;

uint32_t osc_generate(osc_t* osc);
